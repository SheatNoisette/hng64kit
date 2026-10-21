# IOMCU

> @TODO: More research needed
> Only by behavior analysis and dumping what's done in MAME

Toshiba TMP87PH40AN, TLCS-870, 8 Mhz, 32Kb

ROM:
- Sha1: `5b742e8a0bbf4c0ae4f4398d34c7058fb24acc92`
- MAME File: `tmp87ph40an.bin`
- Disasm: `unidasm tmp87ph40an.bin -arch tlcs870 -basepc 0xc000 -skip 0x4000`

## Dual port RAM

The CPU talks to the IOMCU through 2 KB RAM shared by both CPUs
(called DP here). Mapped at `0xBF808000` one byte per address.

CPU writes a command at DP `0x000` and its parameter
at DP `0x001`, kicks the MCU, then reads the status at DP `0x002` and
the inputs.

## Kick

Basically, kick is how the CPU tells the MCU to run the command in DP. Write
a value to the upper half of `0xBF7021C4` (Warning: as 16 bit store).

MCU sets status to `0x02`, reads DP `0x000` and `0x001`,
runs the command and writes the final status to DP `0x002`.

- Poll DP `0x002`, there is no interrupt for the answer apparently ?
- After boot, kick only when the status is `0x00` or `0x01`
- Rewrite DP `0x000` before every kick, the MCU never clears it and
  runs whatever is there

## Boot

From MAME, same as what `roadedge`, `buriki` and `fatfurwa` do:
- Until the MCU is configured, only act on status `0x03` and `0x04`. DP `0x002`
   reads `0x00` at power on and a stale `0x00` for a few us after the first kick
- At status `0x03`, clear DP `0x002` and kick once with command `0x00`. A
   normal boot should have already did it, so the BIOS handshake kicks at status `0x03`
- Do not write to DP until status is `0x04`, the MCU tests the DP and
   locks up for good on any write (only a board reset recovers). It
   takes about 6s in MAME
- At status `0x04`, write `8, 8, 0, 0` to DP `0x200-0x203` (digital,
   analog, output 1, output 2 block sizes) then send command `0x06`. Games
   send command `0x0A` instead
- Send command `0x00` every frame

## Inputs

After each command with status `0x00` or `0x01`, read the player words. Big-endian
16 bit, active high:
| DP            | Player |
|---------------|--------|
| `0x008-0x009` | P1     |
| `0x00A-0x00B` | P2     |

Bits 0-7 and 15 are the stick and buttons (wiring depends on the board io),
bit 14 is Start.

## Commands

| Command | Parameter | Does what |
|---      |---        |---|
| `0x00`  | none      | Refresh inputs, send every frame |
| `0x01`  | ? | ? |
| `0x02`  | ? | ? |
| `0x03`  | ? | ? |
| `0x04`  | ? | ? |
| `0x05`  | ? | ? |
| `0x06`  | none      | Set block sizes from DP `0x200-0x203`, send at status 4 |
| `0x07`  | ? | ? |
| `0x08`  | ? | ? |
| `0x09`  | ? | ? |
| `0x0A`  | ? | ? |
| `0x0B`  | ? | ? |
| `0x0C`  | ? | ? |
| `0x0D`  | ? | ? |
| `0x0E`  | none      | Fw version ? |
| `0x0F`  | ? | ? |

Others seems to do nothing.

## Status codes

Read from DP `0x002`.

| Status | What it is? |
|---     |--- |
| `0x00` | Done, read the inputs |
| `0x01` | Done like `0x00`, plus a coin, service or test event. Read inputs. |
| `0x02` | Busy, never kick |
| `0x03` | Boot, waiting for the first kick |
| `0x04` | Boot done, send command `0x06` |
| `0x05` | ? |
| `0x09` | ? |
| `0x0F` | Error? Detail `0x01` and `0x02` refreshed the inputs, `0x05` and `0x06` mean the MCU is dead ? |
| `0x55`, `0xAA` | Not a status, the MCU's DP test pattern during its boot |

## Misc

String id: `"64Bit I/O Controller Ver 1.0 1997.06.29(C)SNK"`
