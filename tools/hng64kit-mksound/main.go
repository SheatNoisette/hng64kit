// Generate a sound image
//
// Usage: hng64kit-mksound --firmware <soundfw.bin> --out <sound.bin>
package main

import (
	"flag"
	"fmt"
	"os"
)

const (
	// V53A sees 32 blocks of 64 KB and boots from the last one
	blockSize = 0x10000
	imageSize = 32 * blockSize
	fwBlock   = 0x1F

	// Reset lands at FFFF:0000
	resetOffset = 0xFFF0
	farJmp      = 0xEA
)

func loadFirmware(path string) ([]byte, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	if len(data) > blockSize {
		return nil, fmt.Errorf("%s is %d bytes the boot block holds %d", path, len(data), blockSize)
	}

	if len(data) <= resetOffset || data[resetOffset] != farJmp {
		return nil, fmt.Errorf("%s has no far jump at 0x%04X, is it linked with soundfw.lnk?", path, resetOffset)
	}

	return data, nil
}

func buildSound(fwPath, outPath string) error {
	fw, err := loadFirmware(fwPath)
	if err != nil {
		return err
	}

	// File order is V53A order since MAME's sound RAM keeps the MIPS byte order
	img := make([]byte, imageSize)
	copy(img[fwBlock*blockSize:], fw)

	return os.WriteFile(outPath, img, 0o644)
}

func main() {
	fwPath := flag.String("firmware", "", "raw V53A firmware linked with soundfw.lnk")
	outPath := flag.String("out", "", "sound RAM image to write")
	flag.Parse()

	if *fwPath == "" || *outPath == "" || flag.NArg() != 0 {
		fmt.Fprintln(os.Stderr, "usage: hng64kit-mksound --firmware <soundfw.bin> --out <sound.bin>")
		os.Exit(2)
	}

	err := buildSound(*fwPath, *outPath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mksound:", err)
		os.Exit(1)
	}
}
