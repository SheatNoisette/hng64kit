// Command hng64kit-mkrom interleaves a program ROM and writes it as a roadedge ROM set.
//
// Usage: hng64kit-mkrom --program-rom <progrom.bin> --out <rompath>
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
)

// MAME requires all of the chips
var blankChips = []string{
	"001sc01a.41", "001sc02a.42", "001sc03a.43", "001sc04a.44",
	"001sp01a.53", "001sp02a.54", "001sp03a.55", "001sp04a.56",
	"001tx01a.1", "001tx02a.2", "001tx03a.3", "001tx04a.4",
	"001tx01a.5", "001tx02a.6", "001tx03a.7", "001tx04a.8",
	"001tx01a.9", "001tx02a.10", "001tx03a.11", "001tx04a.12",
	"001tx01a.13", "001tx02a.14", "001tx03a.15", "001tx04a.16",
	"001vt01a.17", "001vt02a.18", "001vt03a.19",
	"001sd01a.77", "001sd02a.78",
}

func loadProgRom(path string) ([]byte, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	if len(data) > romSize {
		return nil, fmt.Errorf("%s is %d bytes, the cart holds %d", path, len(data), romSize)
	}

	rom := make([]byte, romSize)
	copy(rom, data)

	return rom, nil
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

func buildRom(progPath, rompath string) error {
	rom, err := loadProgRom(progPath)
	if err != nil {
		return err
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
	rompath := flag.String("out", "", "rompath to write the set into")
	flag.Parse()

	if *progPath == "" || *rompath == "" || flag.NArg() != 0 {
		fmt.Fprintln(os.Stderr, "usage: hng64kit-mkrom --program-rom <progrom.bin> --out <rompath>")
		os.Exit(2)
	}

	err := buildRom(*progPath, *rompath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mkrom:", err)
		os.Exit(1)
	}
}
