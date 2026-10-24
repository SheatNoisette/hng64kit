# Sound Firmware

> @TODO: More research, clean up, infodump for now

Focus on the Buriki One sound driver for now, which is the latest known.

- Trace: `-debug -debugger none` + `manager.machine.debugger:command("trace <fifo>,audiocpu,noloop")`

## Driver versions

| Game     | Version | Label          | String offset |
|----------|---------|----------------|---------------|
| sams64   | 1.00a   | NEO-GEO64      | `$28D1`       |
| roadedge | 1.10    | NEO-GEO64      | `$27E1`       |
| xrally   | 1.10    | HYPER NEOGEO64 | `$FE06`       |
| bbust2   | 1.11    | HYPER NEOGEO64 | `$FE06`       |
| fatfurwa | 1.14    | HYPER NEOGEO64 | `$FE06`       |
| buriki   | 1.15    | HYPER NEOGEO64 | `$FE06`       |

- `xrally` (1.10) and `bbust2` (1.11) have nearly the same fw with an edit on the version with few variables ?

## Extracting the latest driver

This covers 1.15 from `buriki`, the newest version above. The game copies a 2 MB image from the program ROMs to sound RAM at boot.

The image is the deinterleaved program ROM data at 0x600000 - 0x7FFFFF, split in [cartridge.md](./cartridge.md).

You need the first two program ROMs of `buriki`, `007pr01b.81` and `007pr02b.82`. Dump them from your own cart.

Save this as `extract.py`:
```python
import sys

IMG_START = 0x600000
IMG_END = 0x800000

# pr01, bytes 3 and 2 of each word
with open(sys.argv[1], "rb") as f:
    odd = f.read()
# pr02, bytes 1 and 0 of each word
with open(sys.argv[2], "rb") as f:
    even = f.read()

prog = bytearray(len(odd) * 2)
prog[0::4] = even[1::2]
prog[1::4] = even[0::2]
prog[2::4] = odd[1::2]
prog[3::4] = odd[0::2]

img = prog[IMG_START:IMG_END]

with open(sys.argv[3], "wb") as f:
    f.write(img)
```

Run it with Python 3:
```sh
$ python3 extract.py 007pr01b.81 007pr02b.82 buriki-1.15.bin
```

The output should be 2 mb.

Disasm:
```sh
$ unidasm buriki-1.15.bin -arch nec -basepc 0xB54E -skip 0xB54E -count 0x146"
```

## Inner-working

- The firmware is a big beast.
- It seems to be mostly written in assembly.
- x86, 80186 compatible with NEC extensions
- MIDI like? May be a glorified midi sequencer.
- Named internally `NG64SDRV`

### How the FW is loaded?

At boot the game copies 2MB image from the program ROMs to sound RAM at
`0x60200000`, then writes `0x55AA` to `0x6F000000` to start the V53A.

## Firmware

The firmware is a GS-like compatible MIDI sound module with a song sequencer and
a sound effect (SE) player.

The program space is 16 banks of 64 KB and is mapped on `0x00000-0xFFFFF`

| CPU address       | Bank  | Block              | Contents                    |
|-------------------|-------|--------------------|-----------------------------|
| `0x00000-0x0FFFF` | 0     | `$1F`              | Vectors, RAM, code          |
| `0x10000-0x6FFFF` | 1-6   | `$03-$08`          | Instruments?                |
| `0x70000-0x7FFFF` | 7     | `$09`              | Sound directory and SE data |
| `0x80000-0xAFFFF` | 8-10  | `$00-$02` at first | Song window                 |
| `0xB0000-0xFFFFF` | 11-15 | `$1E`              | Nothing                     |

### Protocol

Tap into MAME:
| MIPS address | Direction | V53A port |
|--------------|-----------|-----------|
| `0x68000000` | write     | `0x104`   |
| `0x68000002` | write     | `0x106`   |
| `0x68000006` | read      | `0x102`   |
| `0x68000008` | write     | none ?    |


### Commands

> @TODO: Play all of the game and do a script which compiles everything

`buriki` dump boot sequence:
- `FFFE` (reply `55AA`)
- `0007`
- `0003`
- `067F`
- `34FF` then `345B`
- `0083`
- `0780`
- `0500`
- `0101`
- `057F`
- `2038-2116` during the attract mode

