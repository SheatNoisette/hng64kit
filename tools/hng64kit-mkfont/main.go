// hng64kit-mkfont packs an 8x8 font into SC tiles for the BIOS print
//
// Usage: hng64kit-mkfont --font <font.txt> --out <font.sc.bin>
package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
)

const (
	glyphSize = 8

	// One 8x8 4bpp tile
	tileBytes = 32
	rowBytes  = 4

	// BIOS prints character c as tile c + 8
	firstTile = 8
	charCount = 256

	inkPen = 1

	imageAlign = 256
)

// Row byte holding each pixel pair, MAME's tile layout scrambles them
var pixelPairByte = [rowBytes]int{3, 1, 2, 0}

func parseHeader(line string) (int, error) {
	code, err := strconv.ParseUint(strings.TrimPrefix(line, "U+"), 16, 32)
	if err != nil {
		return 0, fmt.Errorf("bad header %q", line)
	}

	if code >= charCount {
		return 0, fmt.Errorf("%s is past 0xFF, the BIOS prints bytes", line)
	}

	return int(code), nil
}

func parseRow(line string) (byte, error) {
	var bits byte

	if len(line) != glyphSize {
		return 0, fmt.Errorf("row %q isn't %d pixels", line, glyphSize)
	}

	for x := 0; x < glyphSize; x++ {
		switch line[x] {
		case '#':
			bits |= 0x80 >> x
		case '.':
		default:
			return 0, fmt.Errorf("row %q has %q, want # or .", line, line[x])
		}
	}

	return bits, nil
}

func loadFont(path string) (map[int][glyphSize]byte, error) {
	glyphs := map[int][glyphSize]byte{}

	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	lineNo := 0
	code := -1
	var glyph [glyphSize]byte
	row := 0

	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())

		if strings.HasPrefix(line, "U+") {
			if code >= 0 && row != glyphSize {
				return nil, fmt.Errorf("%s:%d: glyph U+%04X has %d rows", path, lineNo, code, row)
			}

			code, err = parseHeader(line)
			if err != nil {
				return nil, fmt.Errorf("%s:%d: %w", path, lineNo, err)
			}

			if _, dup := glyphs[code]; dup {
				return nil, fmt.Errorf("%s:%d: U+%04X twice", path, lineNo, code)
			}

			row = 0
			continue
		}

		if code < 0 || line == "" {
			continue
		}

		if row == glyphSize {
			return nil, fmt.Errorf("%s:%d: glyph U+%04X has more than %d rows", path, lineNo, code, glyphSize)
		}

		glyph[row], err = parseRow(line)
		if err != nil {
			return nil, fmt.Errorf("%s:%d: %w", path, lineNo, err)
		}

		row++
		if row == glyphSize {
			glyphs[code] = glyph
		}
	}

	if err := scanner.Err(); err != nil {
		return nil, err
	}

	if code >= 0 && row != glyphSize {
		return nil, fmt.Errorf("%s: last glyph U+%04X has %d rows", path, code, row)
	}

	return glyphs, nil
}

// High nibble is the left pixel
func encodeRow(bits byte, out []byte) {
	for pair := 0; pair < rowBytes; pair++ {
		left := (bits >> (7 - 2*pair)) & 1
		right := (bits >> (6 - 2*pair)) & 1

		out[pixelPairByte[pair]] = left*inkPen<<4 | right*inkPen
	}
}

func buildImage(glyphs map[int][glyphSize]byte) []byte {
	size := (firstTile + charCount) * tileBytes
	size = (size + imageAlign - 1) / imageAlign * imageAlign
	image := make([]byte, size)

	for code, glyph := range glyphs {
		tile := image[(firstTile+code)*tileBytes:]

		for y := 0; y < glyphSize; y++ {
			encodeRow(glyph[y], tile[y*rowBytes:])
		}
	}

	return image
}

func main() {
	fontPath := flag.String("font", "", "font text file")
	outPath := flag.String("out", "", "SC tile image to write")
	flag.Parse()

	if *fontPath == "" || *outPath == "" || flag.NArg() != 0 {
		fmt.Fprintln(os.Stderr, "usage: hng64kit-mkfont --font <font.txt> --out <font.sc.bin>")
		os.Exit(2)
	}

	glyphs, err := loadFont(*fontPath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mkfont:", err)
		os.Exit(1)
	}

	err = os.WriteFile(*outPath, buildImage(glyphs), 0o644)
	if err != nil {
		fmt.Fprintln(os.Stderr, "hng64kit-mkfont:", err)
		os.Exit(1)
	}
}
