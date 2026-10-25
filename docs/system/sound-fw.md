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

To get as much information on it, it is recommended to tap MAME to each physical
address, patch MAME to print any update to the DSP and disassemble the FW.

#### Communication

| MIPS address | Direction | V53A port | What it is ?                       |
|--------------|-----------|-----------|------------------------------------|
| `0x68000000` | write     | `0x104`   | Command                            |
| `0x68000002` | write     | `0x106`   | Flags (Bit 6 is "command present") |
| `0x68000004` | write     | `0x100`   | Reply or echo                      |
| `0x68000006` | read      | `0x102`   | Status                             |
| `0x68000008` | write     | none ?    | Does a IRQ 5                       |

#### Commands

| Command               | Effect                                                                                                                        |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------|
| `0001 0002 0003`      | ?                                                                                                                             |
| `0005 0006 0007`      | Lock and stop music, SE or both. Locked commands are ignored and does nothing                                                 |
| `0081`                | Stop the music                                                                                                                |
| `0082`                | Stop all SE                                                                                                                   |
| `0083`                | Stop both                                                                                                                     |
| `0084` / `0085`       | Resume / pause the music, pause seems to sends all sound off on 16 channels                                                   |
| `01nn`                | Play song `nn`                                                                                                                |
| `02nn`                | Song tempo ? `nn` BPM ?                                                                                                       |
| `05vv`                | Music volume `vv * 2`, `vv` <= `7F`                                                                                           |
| `06vv`                | SE volume                                                                                                                     |
| `07vv`                | Master volume, `vv` <= `7F`                                                                                                   |
| `07rr`, `rr` ≥ `80`   | Fade speed ? Volumes move by 2 every `(rr − 0x80) * 2` ticks. `0780` is instant ?                                             |
| `1xnn`                | Start SE `nn` of table 1, `nn` + `0x80` stops it. `x` does nothing ?                                                          |
| `2xnn`                | Start SE `x * 0x100 + nn` of table 2, `x` < 8. `2(x+8)nn` stops it                                                            |
| `8xnn`                | Same with `0x800` added                                                                                                       |
| `3???`                | ?                                                                                                                             |
| `4000-7FFF`           | Pitch bend on low 14 bits                                                                                                     |
| `Cgvv`                | Pitch bend on the SE channels of group `g + 1`                                                                                |
| `F000 F001 F002 F004` | Is MIDI mode, `F001` sends the music to MIDI OUT, `F004` SE, `F002` stops sequencer and plays MIDI IN, `F000` is normal mode. |
| `F10x`                | ?                                                                                                                             |
| `FEnn` / `FE80+nn`    | Sound test ?                                                                                                                  |
| `FFF0`                | ?                                                                                                                             |
| `FFF1`                | Reply music volume / 2, SE volume / 2 ?                                                                                       |
| `FFFE`                | Reply `55AA`, handskahe                                                                                                       |
| `FFFF`                | Restart the driver                                                                                                            |
| others                | Invalid                                                                                                                       |


#### Example of a communication

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

