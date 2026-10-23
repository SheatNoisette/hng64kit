# Feasability of a custom flashcart

> @TODO: Research on the cart, get a motherboard and a game + logic analyzer

> Infodump for now, please do not use this info

- Chip Toshiba TC5332200AF
- Mask ROM (CMOS), factory burned
- SOP44 / PDSO44
- 120 ns max
- Datasheet: https://datasheet.datasheetarchive.com/originals/scans/Scans-056/DSAIH000125594.pdf
    - Single 5 V supply, 4.5 to 5.5 V
    - TTL inputs and outputs, three-state
    - Address to data 120 ns max, OE to data 60 ns max, hold 5 ns min
    - SOP44 600 mil, pin 1 is NC
    - Pins A0-A20, D0-D14, D15/A-1, CE, OE, BYTE
    - BYTE high: 16-bit, D15/A-1 is D15
    - BYTE low: 8-bit, D15/A-1 is the lowest address bit
- HNG64 master clock is 33.333 MHz
    - 30 ns per cycle
    - NOR flash at 70 to 90 ns returns data in 3 cycles
- TX group is four identical copies
    - Bandwidth reasons

## To verify

- 3v3 or 5V ?
    - Has a 3v3 regulator, guess 3v3 ?

## Chips

- Count of chips to fill one bus width
- Widths [cartridge.md](cartridge.md) + MAME
- TC5332200AF is 4M x 8 or 2M x 16
- A x16 chip divide count

| Bank | Width (bits)          | Chips (×8 mode) | Chips (×16 mode)        |
|------|-----------------------|-----------------|-------------------------|
| `PR` | 32                    | 4               | 2                       |
| `SC` | 32 (two 16-bit banks) | 4               | 2                       |
| `SP` | 32                    | 4               | 2                       |
| `TX` | 8 (per copy)          | 1               | 1 (half the bus unused) |
| `VT` | 48                    | 6               | 3                       |
| `SD` | 8                     | 1               | 1 (half the bus unused) |

- Random access no burst
- Bandwidth = bytes per access / access time
- Mask ROM 120 ns max, 8.33 M accesses/s
- NOR flash 90 ns worst case, 11.11 M accesses/s
- Master clock cycle is 30 ns
    - Mask ROM: 120 / 30 = 4 cycles
    - NOR flash: 90 / 30 = 3 cycles
- Example `PR` at 120 ns: 4 B / 120 ns = 33.3 MB/s
- Example `VT` at 90 ns: 6 B / 90 ns = 66.7 MB/s
- `TX` 4 identical copies, so total is 4x one copy ?

| Bank | Bytes per access  | Mask ROM, 120 ns (MB/s) | NOR flash, 90 ns (MB/s) |
|------|-------------------|-------------------------|-------------------------|
| `PR` | 4                 | 33.3                    | 44.4                    |
| `SC` | 4                 | 33.3                    | 44.4                    |
| `SP` | 4                 | 33.3                    | 44.4                    |
| `TX` | 1 (x4 copies = 4) | 8.3 (x4 = 33.3)         | 11.1 (x4 = 44.4)        |
| `VT` | 6                 | 50.0                    | 66.7                    |
| `SD` | 1                 | 8.3                     | 11.1                    |

Note:
- PSRAM doesn't make the cut with stalls and random RW

## Ideas

Few ideas to make a flashcart.

With connector, dumb idea:
Custom interposer and solder each connection to a dupont breakout under the
connector. Dangerous, bad wiring will crash the board, crosstalk with long wire
and timing issues.

For quick testing for the SDK, only the `PR` should be good enough to write
a custom software and reuse asset of the game. Boot in fast mode to test as much
custom code as possible.

### NOR Cart

How:
- Take a game, put a socket on every chip, reflash at each game

Pro:
- Cheap
- Use real cart connector

Cons:
- Risks of destroying a game and cart
- Need a adapter + socket

Potential roms:
- Macronix MX29LV320E
- Infineon/Spansion S29AL032D
- /!\ EOL Micron M29W320


### FPGA

How:
- Put an FPGA board on the cart connector, no mask ROMs
- It answers like ROM -> address in, data out, no wait line
- Game data sits in flash or a MCU on USB connected on a puter
- Copy it to RAM at poweron
- Every read must finish in 4 cycles (120 ns)

Pro:
- No game or cart to destroy
- Switch games without reflashing (Insane for dev)
- Bank widths and chip counts stay in the FPGA, not on the PCB

Cons:
- Hart to design
- Needs a lot of RAM, block RAM is far too small
- Must serve random reads in 120 ns
- SDRAM : Stalls and refresh IP
- Many pins, address and data for every bank
- Connector pinout is not mapped yet, may need to solder everything using cables
- Expensive af
