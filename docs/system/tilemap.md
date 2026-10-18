# Tilemaps

> Based largely on MAME's `hng64_v.cpp` and the BIOS
> Bios text init at `0xBFC08148`

The tilemap chip draws 8x8 or 16x16 tiles from the cart `SC` ROM. RAM
holds tile numbers, registers and the palette, so there is no bitmap mode.

Addresses go through TLB window `0xC0xxxxxx` = physical `0x20xxxxxx`,
uncached. BIOS sets it up on a normal boot and `hng_core_init` does it again.

## Memory

| Address      | What ? |
|---           |---|
| `0xC0100000` | 4 maps of 128 x 128 32-bit entries, map `n` at `+ n * 0x10000` rows of 128 |
| `0xC0140000` | Scroll and zoom records, ignored while zoom is off |
| `0xC0190000` | Tilemap registers |
| `0xC0200000` | Palette, 4096 x `0x00RRGGBB` |
| `0xC0208000` | Mixer  |
| `0xD0000000` | 3D control bytes (window `0xD0xxxxxx` = `0x30xxxxxx`) |

## Entry

`pppppppp ffattttt tttttttt tttttttt`

| Bits  | Use |
|---    |---|
| 31-24 | Palette of 4bpp: 256 palettes of 16 colors, color `n` is entry `p * 16 + n` |
| 23-22 | Flip y, flip x |
| 21    | Auto animate (ignored unless register `0x04` bit 16 is set) |
| 20-0  | Tile number in 32-byte units, one 8x8 4bpp tile |

Pixel 0 is transparent.

## Registers (`0xC0190000`)

| Offset | Text value | What |
|---|---|---|
| `0x00` | `0x00070000` | Bit 16 turns zoom and scroll off, all maps are drawn 1:1 from 0,0. Bits 17-18 always set ? |
| `0x08` | `0x08400000` | Layer 0 flags (high half), layer 1 (low half) |
| `0x0C` | `0x00000000` | Layers 2 and 3 |
| `0x10` | `0x00000000` | Scroll record base, layers 0 and 1 |

Layer flags, `mmmm dbrW FELz zzzz`:

| Bits  | What it does? |
|---    |---|
| 15-12 | Mosaic |
| 11    | 1 = normal, 0 = line mode |
| 10    | 8bpp |
| 9     | 16x16 tiles |
| 8     | Wrap |
| 7     | Fade enable ? |
| 6     | Enable |
| 5     | Mixer group ? |
| 4-0   | Priority, `0x10-0x1F` behind 3D |

`0x0840` is 8x8 4bpp, enabled, priority 0, the BIOS text layer.

## Mixer

| Address               | Value | Why |
|---                    |---    |---  |
| `0xC0208000` (16-bit) | `0x0007`     | BIOS value, unknown |
| `0xC0208004`          | `0x00000000` | Window min x (high), min y (low) |
| `0xC0208008`          | `0x020001C0` | Window 512 x 448, a zero blanks the screen |
| `0xC020800C`          | `0x01404209` | BIOS value, MAME reads blend modes from bits 2 and 26 |
| `0xC0208018`, `0xC020801C`     | `0` | Fades off |
| `0xC0208024`          | `0`          | Palette effects off, bit 17 hides everything (MAME) |
| `0xD0000000` (8-bit)  | `1`          | 3D layer off, backdrop is palette entry 0 (MAME) |

## SC ROM, 8x8 4bpp

From `roadedge`. Each pair is one 16-bit bank, lower number on the even bytes:
bank A is `SC01`/`SC02`, bank B is `SC03`/`SC04`.

Tile `u`, row `y`:
- bank B if `u` is even, bank A if odd
- `o = 16 * (u >> 1) + 2 * y` in both chips of the bank
- pixels 0 to 7 are `odd[o+1]` high, low, `odd[o]` high, low, `even[o+1]` high, low, `even[o]` high, low

A 4 MB chip pair holds 512K tiles. Pixel order comes from MAME's
`hng64_reorder` and `hng64_8x8x4_tilelayout`

## BIOS font

The BIOS has no font. It prints character `c` as `(palette << 24) | (c + 8)`,
with palette entry 1 white and `0x11` red,
text comes from the cart.
