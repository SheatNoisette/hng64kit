// Command hng64kit-mkprogrom fills the cart header of an hng64kit ELF and writes it as a linear program ROM.
//
// Usage: hng64kit-mkprogrom --elf <program.elf> [--name <title>] --out <progrom.bin>
package main

import (
	"debug/elf"
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"os"
)

const (
	headerSize = 0x80

	headerTitle     = 0x00
	headerBootFlags = 0x24
	headerLoadSrc   = 0x50
	headerLoadDest  = 0x54
	headerLoadWords = 0x58
	headerEntry     = 0x5C
	headerMagic     = 0x60

	titleSize = 32

	// Quiet boot by default
	bootQuiet = 0x4000

	magic = "HYPER NEOGEO64"

	cartRomPhys = 0x04000000
	physMask    = 0x1FFFFFFF

	// Roadedge has a single pr pair of 32 Mbit chips
	romSize = 0x800000
)

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

func fillHeader(rom []byte, name string, start, end, entry uint64) error {
	for _, b := range rom[:headerSize] {
		if b != 0 {
			return fmt.Errorf("header space isn't empty crt0 must come first")
		}
	}

	if len(name) > titleSize {
		return fmt.Errorf("name is %d bytes, the header only holds %d", len(name), titleSize)
	}

	be := binary.BigEndian
	copy(rom[headerTitle:], name)
	be.PutUint16(rom[headerBootFlags:], bootQuiet)
	be.PutUint32(rom[headerLoadSrc:], cartRomPhys)
	be.PutUint32(rom[headerLoadDest:], uint32(start&physMask))
	be.PutUint32(rom[headerLoadWords:], uint32((end-start)/4))
	be.PutUint32(rom[headerEntry:], uint32(entry))
	copy(rom[headerMagic:], magic)

	return nil
}

func buildProgRom(elfPath, name, outPath string) error {
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

	err = fillHeader(rom, name, start, end, f.Entry)
	if err != nil {
		return err
	}

	// Only the loaded words matter, mkrom pads the rest anyway
	image := rom[:end-start]

	return os.WriteFile(outPath, image, 0o644)
}

func main() {
	elfPath := flag.String("elf", "", "program ELF linked with hng64.ld")
	name := flag.String("name", "", "title written in the cart header")
	outPath := flag.String("out", "", "program ROM to write")
	flag.Parse()

	if *elfPath == "" || *outPath == "" || flag.NArg() != 0 {
		fmt.Fprintln(os.Stderr, "usage: hng64kit-mkprogrom --elf <program.elf> [--name <title>] --out <progrom.bin>")
		os.Exit(2)
	}

	err := buildProgRom(*elfPath, *name, *outPath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mkprogrom:", err)
		os.Exit(1)
	}
}
