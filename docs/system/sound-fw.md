# Sound Firmware

Focus on the Buriki One sound driver.

## Driver versions

| Game     | Version| Label          | String offset |
|---       |---     |---             |---            |
| sams64   | 1.00a  | NEO-GEO64      | `$28D1` |
| roadedge | 1.10   | NEO-GEO64      | `$27E1` |
| xrally   | 1.10   | HYPER NEOGEO64 | `$FE06` |
| bbust2   | 1.11   | HYPER NEOGEO64 | `$FE06` |
| fatfurwa | 1.14   | HYPER NEOGEO64 | `$FE06` |
| buriki   | 1.15   | HYPER NEOGEO64 | `$FE06` |

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
