// Command hng64kit-mkrom fills the cart header of an hng64kit ELF and writes it as a roadedge ROM set.
//
// Usage: hng64kit-mkrom <program.elf> <rompath>
// @TODO: Not hardcoded to roadedge
package main

import (
	"debug/elf"
	"encoding/binary"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

const (
	headerSize = 0x80

	headerBootFlags = 0x24
	headerLoadSrc   = 0x50
	headerLoadDest  = 0x54
	headerLoadWords = 0x58
	headerEntry     = 0x5C
	headerMagic     = 0x60

	// Quiet boot by default
	bootQuiet = 0x4000

	magic = "HYPER NEOGEO64"

	cartRomPhys = 0x04000000
	physMask    = 0x1FFFFFFF

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

func findSymbol(f *elf.File, name string) (uint64, error) {
	syms, err := f.Symbols()
	if err != nil {
		return 0, err
	}

	for _, s := range syms {
		if s.Name == name {
			return s.Value, nil
		}
	}

	return 0, fmt.Errorf("symbol %s not found, is the program linked with hng64.ld?", name)
}

func loadImage(f *elf.File, start uint64) ([]byte, error) {
	rom := make([]byte, romSize)

	for _, p := range f.Progs {
		if p.Type != elf.PT_LOAD || p.Filesz == 0 {
			continue
		}

		offset := p.Paddr - start
		if p.Paddr < start || offset+p.Filesz > romSize {
			return nil, fmt.Errorf("segment at 0x%08X doesn't fit in the %d MB cart", p.Paddr, romSize>>20)
		}

		_, err := io.ReadFull(p.Open(), rom[offset:offset+p.Filesz])
		if err != nil {
			return nil, err
		}
	}

	return rom, nil
}

func fillHeader(rom []byte, start, end, entry uint64) error {
	for _, b := range rom[:headerSize] {
		if b != 0 {
			return fmt.Errorf("header space isn't empty, crt0 must come first")
		}
	}

	be := binary.BigEndian
	be.PutUint16(rom[headerBootFlags:], bootQuiet)
	be.PutUint32(rom[headerLoadSrc:], cartRomPhys)
	be.PutUint32(rom[headerLoadDest:], uint32(start&physMask))
	be.PutUint32(rom[headerLoadWords:], uint32((end-start)/4))
	be.PutUint32(rom[headerEntry:], uint32(entry))
	copy(rom[headerMagic:], magic)

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

func buildRom(elfPath, rompath string) error {
	f, err := elf.Open(elfPath)
	if err != nil {
		return err
	}
	defer f.Close()

	if f.Machine != elf.EM_MIPS || f.ByteOrder != binary.BigEndian {
		return fmt.Errorf("%s isn't a big endian mips elf", elfPath)
	}

	start, err := findSymbol(f, "__image_start")
	if err != nil {
		return err
	}

	end, err := findSymbol(f, "__image_end")
	if err != nil {
		return err
	}

	rom, err := loadImage(f, start)
	if err != nil {
		return err
	}

	err = fillHeader(rom, start, end, f.Entry)
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
	if len(os.Args) != 3 {
		fmt.Fprintln(os.Stderr, "usage: hng64kit-mkrom <program.elf> <rompath>")
		os.Exit(2)
	}

	err := buildRom(os.Args[1], os.Args[2])
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mkrom:", err)
		os.Exit(1)
	}
}
