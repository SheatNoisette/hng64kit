"""
Usage: uv run progroms2bin.py --out <rom.bin> <pr01> <pr02> [<pr03> <pr04> ...]
"""

import argparse
import sys

# MAME loads every PR chip as 32 Mbit
CHIP_SIZE = 0x400000
PAIR_SIZE = 2 * CHIP_SIZE

def read_chip(path):
    with open(path, "rb") as f:
        chip = f.read()

    if len(chip) != CHIP_SIZE:
        raise ValueError(f"{path} is {len(chip)} bytes, a PR chip is {CHIP_SIZE}")

    return chip

def merge_pair(first, second):
    pair = bytearray(PAIR_SIZE)

    pair[0::4] = second[1::2]
    pair[1::4] = second[0::2]
    pair[2::4] = first[1::2]
    pair[3::4] = first[0::2]

    return pair

def merge_chips(paths):
    rom = bytearray()

    for i in range(0, len(paths), 2):
        first = read_chip(paths[i])
        second = read_chip(paths[i + 1])
        rom += merge_pair(first, second)

    return rom

def main():
    parser = argparse.ArgumentParser(description="Merge PR chip pairs into one linear program ROM")
    parser.add_argument("--out", required=True, help="linear program ROM to write")
    parser.add_argument("chips", nargs="+", help="PR chips in board order, pr01 first")
    args = parser.parse_args()

    if len(args.chips) % 2 != 0:
        print(f"progroms2bin: got {len(args.chips)} chips, PR chips come in pairs", file=sys.stderr)
        return 1

    try:
        rom = merge_chips(args.chips)
    except ValueError as err:
        print(f"progroms2bin: {err}", file=sys.stderr)
        return 1

    with open(args.out, "wb") as f:
        f.write(rom)

    return 0

if __name__ == "__main__":
    sys.exit(main())
