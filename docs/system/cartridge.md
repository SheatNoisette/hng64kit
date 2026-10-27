# Hyper Neo Geo 64 Cartidge format

The cart is a passive board of 32 Mbit (4 MB) or 64 Mbit (8 MB) mask ROMs:
- LVS-DG1 (1997): all ROMs 32 Mbit
- LVS-DG2 (1998): SC and SP ROMs are 64 Mbit

It is split into multiple types:
| Type | Name                   | How MAME loads it ?          | Who reads this? | Max on board in MB |
|---   |---                     |---                           |---              |---                 |
| `PR` | Program                | 32-bit, pairs of 16-bit ROMs | Main CPU, mapped `0x04000000` | 16 positions (MAME maps 32 MB ?) |
| `SC` | Scroll (tilemap) tiles | 16-bit pairs, two banks read in parallel | Tilemap chip | 12 (DG1) / 8 (DG2) |
| `SP` | Sprite tiles           | 32-bit, groups of 4 ROMs      | Sprite chip | 24 (DG1) / 16 (DG2) |
| `TX` | Textures               | 8-bit, 4 ROMs per copy, 4 identical copies  | 3D renderer | 4 x 4 |
| `VT` | Vertex / model data    | 48-bit, groups of 3 ROMs                    | 3D renderer | 24 |
| `SD` | Sound samples          | 8-bit, sequential             | L7A1045 DSP | 4 |

## Connector format

This is a quick overview of the cartidge connectors, a Kicad is a better fit for
a schematic.

- The connector is made by HRS (Hirose Electric), may be End-Of-Life or custom.
- 300 pins per connector, splitted in six rows of 25 pins, two groups of 150
  (6 x 25) pins, separated by a screw hole (|150|Screw|150|)
- Contact layout is double row
- The connectors on the motherboard is the "female" variant of this connector
- Nothing is marked on it except "300" / "HRS"
- Connector size: 12.5mm x 140.5mm (Group 1 + Screw + Group 2)
    - Group size: 66.5 mm
- Size of the PCB support footprint flat rectangle: 15.8mm x 153.6mm
- Split view, One group, looking at top: |A B|C D|
    - A: 50 Pins - Pitch 0.5 mm
    - B: 25 Pins - Pitch 1.7 mm
    - C: 25 Pins - Pitch 1.7 mm
    - D: 50 Pins - Pitch 0.5 mm
- The spacing between the two groups, where the screw is, is 7.5 mm
- Through hole, soldered through the PCB.
- Two on the motherboard, labelled CON9 / CON10. They are identical.
- Squares on the silk PCB are not necessarily ground
- (Guess) Connected to CON9: NEO64-REN (ASIC1/ASIC2), NEO64-TRI2, NEO64-GTE
- (Guess) Connected to CON10: NEO64-BGC, NEO64-SPR, NEO64-SCC, L7A1045

## ROM

Each ROM file stores its 16-bit words little endian.

The ROM is interleaved and big endian, tldr:
```py
def split_program_rom(img):

    odd = bytearray(len(img) // 2)    # pr01, loaded at +2
    even = bytearray(len(img) // 2)   # pr02, loaded at +0

    even[0::2] = img[1::4]
    even[1::2] = img[0::4]
    odd[0::2] = img[3::4]
    odd[1::2] = img[2::4]

    return odd, even
```

## Header

| Offset | Type   | Field | Read by |
|---     |---     |---     |---|
| `0x00` | u8[32] | Title, NUL or space padded | |
| `0x20` | u16    | Game number (0x0001 = 001...) |  |
| `0x22` | u16    | Unknown ? |  |
| `0x24` | u16    | Boot flags | boot block, system block |
| `0x26` | u16    | Unknown, always 0 ? | |
| `0x28` | u16    | System block version. 0 skips the check | boot block |
| `0x2A` | u16    | System block checksum | boot block |
| `0x2C` | u32    | System block source address (kseg1). Length is fixed at 0x2000 | flasher |
| `0x30` | u16    | Service block version. 0 skips the check | boot block |
| `0x32` | u16    | Service block checksum | boot block |
| `0x34` | u32    | Service block source address | flasher |
| `0x38` | u32    | Service block length in bytes | boot block, flasher |
| `0x3C` | u16    | Network boot block version, sent to the network CPU | system block |
| `0x3E` | u16    | Network boot block checksum (only sent if 0x3C != 0 ?) | system block |
| `0x40` | u32    | Network boot block source. Length is fixed at `0x2000` | system block |
| `0x44` | u16    | Network program version | system block |
| `0x46` | u16    | Network program checksum (only sent if 0x44 != 0) | system block |
| `0x48` | u32    | Network program source | System block |
| `0x4C` | u32    | Network program length in bytes | system block |
| `0x50` | u32    | Load source, physical address | boot or system block |
| `0x54` | u32    | Load destination, physical address | Boot or system block |
| `0x58` | u32    | Load length in 32-bit words | boot or system block |
| `0x5C` | u32    | Entry point, virt address | boot or system block |
| `0x60` | u8[16] | Magic `HYPER NEOGEO64` | boot block |
| `0x70` | u16[6] | ROM sizes in Mbit: PR, TX, SC, SP, VT, SD ? |  |
| `0x7C` | u32    | Always 0 ? | |

### Checksum

Seems to be a 16-bit sum of bytes, `sum(block) & 0xFFFF`
(See BIOS addr `0xBFC002C4`). The service block checksum covers `[0x38]` bytes
from BIOS `0x8000`.
