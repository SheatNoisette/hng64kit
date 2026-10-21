# Text

The BIOS can print text on tilemap 0, but it ships without a font by default.

## Printing

```c
#include <libhng64/bios.h>
#include <libhng64/core.h>

int main(void)
{
    hng_core_init();
    hng_bios_text_init();

    hng_bios_print("HELLO GUYS", 2, 2, HNG_BIOS_PALETTE_WHITE);
    hng_bios_print("Yea", 3, 2, HNG_BIOS_PALETTE_RED);

    for (;;) {
    }
}
```

- `hng_bios_text_init` wipes the palette and sets tilemap 0 up as 512x448 text,
  64 columns by 56 rows of 8x8 cells
- `hng_bios_print(text, row, col, palette)` writes one cell per byte until the
  `\0`, rows and columns count cells from the top left
- Printing over old text only replaces the cells it writes, pad numbers to a
  fixed width so a shorter one hides the old digits
- Both are BIOS routines, so they need a normal boot (the kit default) and
  interrupts off, see [BIOS](../system/bios.md)

See `examples/hello` for an example if needed

## The font

`tools/hng64kit-mkfont/font.txt` holds the printable ASCII glyphs of
[funscii](https://github.com/jdebp/funscii) (CC0 1.0). Each glyph is a
`U+XXXX` line followed by 8 rows of 8 pixels, `#` set and `.` clear:

```
U+0041
...##...
..####..
.##..##.
.##..##.
.######.
.##..##.
.##..##.
........
```

To use another font, point `HNG64KIT_FONT` at it before calling
`hng64_add_rom`:
```cmake
set(HNG64KIT_FONT "${CMAKE_CURRENT_SOURCE_DIR}/myfont.txt")
hng64_add_rom(hello)
```

## Tools

Example:
```sh
hng64kit-mkfont --font font.txt --out font.sc.bin
hng64kit-mkrom --program-rom hello.prog.bin --sc font.sc.bin --out roms
```

