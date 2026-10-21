// Command hng64kit-mkrom interleaves a program ROM and writes it as a roadedge ROM set.
//
// Usage: hng64kit-mkrom --program-rom <progrom.bin> [--sound <sound.bin>] [--sc <sc.bin>] --out <rompath>
// @TODO: Not hardcoded to roadedge
package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

const (
	// Roadedge has a single pr pair of 32 Mbit chips
	chipSize = 0x400000
	romSize  = 2 * chipSize

	gameName = "roadedge"
	oddChip  = "001pr01b.81"
	evenChip = "001pr02b.82"

	soundOffset = 0x600000
	soundSize   = 0x200000

	// Tile units, MAME hands out even ones from bank B and odd ones from bank A
	scUnitBytes = 32
	scSize      = 4 * chipSize
)

// Lower number holds the even bytes of each bank
var scChips = [4]string{"001sc01a.41", "001sc02a.42", "001sc03a.43", "001sc04a.44"}

// MAME requires all of the chips
var blankChips = []string{
	"001sp01a.53", "001sp02a.54", "001sp03a.55", "001sp04a.56",
	"001tx01a.1", "001tx02a.2", "001tx03a.3", "001tx04a.4",
	"001tx01a.5", "001tx02a.6", "001tx03a.7", "001tx04a.8",
	"001tx01a.9", "001tx02a.10", "001tx03a.11", "001tx04a.12",
	"001tx01a.13", "001tx02a.14", "001tx03a.15", "001tx04a.16",
	"001vt01a.17", "001vt02a.18", "001vt03a.19",
	"001sd01a.77", "001sd02a.78",
}

func loadProgRom(path string, limit int) ([]byte, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	if len(data) > limit {
		return nil, fmt.Errorf("%s is %d bytes, the program space holds %d", path, len(data), limit)
	}

	rom := make([]byte, romSize)
	copy(rom, data)

	return rom, nil
}

func placeSound(rom []byte, path string) error {
	data, err := os.ReadFile(path)
	if err != nil {
		return err
	}

	if len(data) != soundSize {
		return fmt.Errorf("%s is %d bytes, a sound image is %d", path, len(data), soundSize)
	}

	copy(rom[soundOffset:], data)

	return nil
}

// MAME swaps each chip's 16-bit words so the even chip holds b1 b0 and the odd one b3 b2
func splitRom(rom []byte) (odd, even []byte) {
	odd = make([]byte, chipSize)
	even = make([]byte, chipSize)

	for i := 0; i < chipSize/2; i++ {
		word := rom[i*4 : i*4+4]

		even[i*2] = word[1]
		even[i*2+1] = word[0]
		odd[i*2] = word[3]
		odd[i*2+1] = word[2]
	}

	return odd, even
}

func loadSc(path string) ([]byte, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	if len(data) > scSize {
		return nil, fmt.Errorf("%s is %d bytes, the SC chips hold %d", path, len(data), scSize)
	}

	sc := make([]byte, scSize)
	copy(sc, data)

	return sc, nil
}

func splitSc(sc []byte) [4][]byte {
	var chips [4][]byte

	for i := range chips {
		chips[i] = make([]byte, chipSize)
	}

	for unit := 0; unit < scSize/scUnitBytes; unit++ {
		bank := 2
		if unit%2 == 1 {
			bank = 0
		}

		src := sc[unit*scUnitBytes : (unit+1)*scUnitBytes]
		base := unit / 2 * scUnitBytes

		for k, b := range src {
			offset := (base + k) / 2
			lane := (base + k) % 2

			chips[bank+lane][offset] = b
		}
	}

	return chips
}

func writeSc(scPath, dir string) error {
	if scPath == "" {
		for _, name := range scChips {
			err := writeBlankChip(filepath.Join(dir, name))
			if err != nil {
				return err
			}
		}

		return nil
	}

	sc, err := loadSc(scPath)
	if err != nil {
		return err
	}

	chips := splitSc(sc)
	for i, name := range scChips {
		err = os.WriteFile(filepath.Join(dir, name), chips[i], 0o644)
		if err != nil {
			return err
		}
	}

	return nil
}

// truncate keeps the file sparse so 116 MB of zeros costs nothing on disk
func writeBlankChip(path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}

	err = f.Truncate(chipSize)
	if err != nil {
		f.Close()
		return err
	}

	return f.Close()
}

func buildRom(progPath, soundPath, scPath, rompath string) error {
	limit := romSize
	if soundPath != "" {
		limit = soundOffset
	}

	rom, err := loadProgRom(progPath, limit)
	if err != nil {
		return err
	}

	if soundPath != "" {
		err = placeSound(rom, soundPath)
		if err != nil {
			return err
		}
	}

	odd, even := splitRom(rom)
	dir := filepath.Join(rompath, gameName)

	err = os.MkdirAll(dir, 0o755)
	if err != nil {
		return err
	}

	err = os.WriteFile(filepath.Join(dir, oddChip), odd, 0o644)
	if err != nil {
		return err
	}

	err = os.WriteFile(filepath.Join(dir, evenChip), even, 0o644)
	if err != nil {
		return err
	}

	err = writeSc(scPath, dir)
	if err != nil {
		return err
	}

	for _, name := range blankChips {
		err = writeBlankChip(filepath.Join(dir, name))
		if err != nil {
			return err
		}
	}

	return nil
}

func main() {
	progPath := flag.String("program-rom", "", "program ROM from hng64kit-mkprogrom")
	soundPath := flag.String("sound", "", "sound RAM image from hng64kit-mksound")
	scPath := flag.String("sc", "", "SC tile image from hng64kit-mkfont")
	rompath := flag.String("out", "", "rompath to write the set into")
	flag.Parse()

	if *progPath == "" || *rompath == "" || flag.NArg() != 0 {
		fmt.Fprintln(os.Stderr, "usage: hng64kit-mkrom --program-rom <progrom.bin> [--sound <sound.bin>] [--sc <sc.bin>] --out <rompath>")
		os.Exit(2)
	}

	err := buildRom(*progPath, *soundPath, *scPath, *rompath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mkrom:", err)
		os.Exit(1)
	}
}
