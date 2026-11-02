# Sprites

> /!\ Based on MAME's `hng64_sprite.ipp` and what Roadedge writes
> /!\ Sections seems bogus

> Status: Draft

The sprite chip draws 16x16 tiles from the cart `SP` ROM. RAM holds one 32-byte
entry per sprite plus a few global registers. Colors come from the same palette
as the tilemaps.

Addresses go through TLB window `0xC0xxxxxx` = physical `0x20xxxxxx`,
uncached (?),like the tilemaps.

## Memory

| Address      | What                                                   |
|--------------|--------------------------------------------------------|
| `0xC0000000` | Sprite RAM, 1536 entries of 8 words                    |
| `0xC0010000` | Sprite registers                                       |
| `0xC0200000` | Palette, 4096 x `0x00RRGGBB`, shared with the tilemaps |

## Entry

| Word | Bits        | Use                                                         |
|------|-------------|-------------------------------------------------------------|
| 0    | 31-16, 15-0 | Y, X. Signed 10 bits, 0,0 is the top left of the screen     |
| 1    | 31-16, 15-0 | Y zoom, X zoom. `0x0100` is 1:1, a zero hides the sprite    |
| 2    | 26-16       | Depth (Z), 11 bits                                          |
| 2    | 8           | Inline chain, tiles come from the following entries         |
| 2    | 7-4, 3-0    | Chain width and height minus one, up to 16 x 16 tiles       |
| 3    | 23-16       | Palette of 4bpp: color `n` is entry `p * 16 + n`            |
| 4    | 31-28       | Mosaic                                                      |
| 4    | 26          | Checkerboard, every other pixel (shadows)                   |
| 4    | 25, 24      | Flip x, flip y                                              |
| 4    | 23          | Blend                                                       |
| 4    | 22-20       | Group, where the sprite sits between the tilemap priorities |
| 4    | 18-0        | Tile number in 128-byte units, one 16x16 4bpp tile          |
| 5-7  |             | Unused, games leave them zero                               |

Pen 0 seems transparent

## Registers (`0xC0010000`)

| Offset | Value        | What                                                                |
|--------|--------------|---------------------------------------------------------------------|
| `0x00` | `0x01860000` | Bit 24 smaller depth in front, bit 23 4bpp, bits 18-17 always set ? |
| `0x04` | `0x00000000` | Global offset, y high, x low                                        |
| `0x08` | `0x00000000` | Unknown, `0x000FFFFF` in other games                                |
| `0x0C` | `0x00000000` | Unknown                                                             |
| `0x10` | `0x000000E0` | Unknown, `0x01C0` in `sams64`                                       |

The BIOS text init leaves `0x00070000` at 0x0

Bit 23 is global so every sprite on screen is 4bpp or every one is 8bpp.

## Depth

With bit 24 set, a smaller depth is in front and depth `0x7FF` is never drawn.
`roadedge`, `xrally`, `fatfurwa` and `buriki` retire entries by writing
`0x07FF0000` to word 2. With bit 24 clear it is the other way round: larger in
front, 0 never drawn.

BIOS POST leaves `0xAAAAAAAA` every word, which draws tile `0x2AAAA` at
-342,-342, so hide every entry before use (@TODO: Verify it's a problem in hw)

## How games draw

Games keep a copy of the entries in main RAM, change it during the frame and
copy it to sprite RAM in vblank. Registers get the same thing.

## SP ROM, 16x16 4bpp

MAME loads the four chips `SP01` to `SP04` 32-bit byte interleaved: byte
`4k + c` of the sprite space is byte `k` of `SP0(c + 1)`. (@TODO: Cart?)

Tile `n` is 128 bytes at `n * 128`, 16 rows of 8 bytes `B0` to `B7`. Pixels 0
to 15 of a row are the high then low nibble of `B7 B3 B6 B2 B5 B1 B4 B0`.

A chain takes tiles `n`, `n + 1`, ... left to right, then down

8bpp tiles are 256 bytes, the chip drops bit 0 of the tile number
