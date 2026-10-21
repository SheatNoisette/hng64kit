"""
Usage: uv run progrom2elf.py --rom <progrom.bin> --out <program.elf>
"""

import argparse
import struct
import sys

from elftools.construct.lib import Container
from elftools.elf.constants import E_FLAGS, P_FLAGS, SH_FLAGS
from elftools.elf.structs import ELFStructs

HEADER_SIZE = 0x80

HEADER_LOAD_SRC = 0x50
HEADER_LOAD_DEST = 0x54
HEADER_LOAD_WORDS = 0x58
HEADER_ENTRY = 0x5C
HEADER_MAGIC = 0x60

MAGIC = b"HYPER NEOGEO64"

CART_ROM_PHYS = 0x04000000
KSEG0 = 0x80000000
WORD_SIZE = 4

# VR4300 is MIPS III + o32 matches what the SDK toolchain emits
EF_MIPS_ABI_O32 = 0x1000
ELF_FLAGS = E_FLAGS.EF_MIPS_ARCH_3 | EF_MIPS_ABI_O32

EHDR_SIZE = 52
PHDR_SIZE = 32
SHDR_SIZE = 40

SHSTRTAB = b"\0.text\0.shstrtab\0"
TEXT_NAME = SHSTRTAB.index(b".text")
SHSTRTAB_NAME = SHSTRTAB.index(b".shstrtab")

RWX_SEGMENT = P_FLAGS.PF_R | P_FLAGS.PF_W | P_FLAGS.PF_X
RWX_SECTION = SH_FLAGS.SHF_ALLOC | SH_FLAGS.SHF_WRITE | SH_FLAGS.SHF_EXECINSTR

def read_load_info(rom):
    if len(rom) < HEADER_SIZE:
        raise ValueError(f"ROM is {len(rom)} bytes, too short for the cart header")

    if rom[HEADER_MAGIC:HEADER_MAGIC + len(MAGIC)] != MAGIC:
        raise ValueError("no HYPER NEOGEO64 magic, is this a linear program ROM?")

    src, dest, words, entry = struct.unpack_from(">IIII", rom, HEADER_LOAD_SRC)
    offset = src - CART_ROM_PHYS
    size = words * WORD_SIZE

    if src < CART_ROM_PHYS or offset + size > len(rom):
        raise ValueError(f"load of 0x{size:X} bytes from 0x{src:08X} runs past the {len(rom)} byte ROM")

    if dest % WORD_SIZE != 0:
        raise ValueError(f"load destination 0x{dest:08X} isn't word aligned")

    return rom[offset:offset + size], KSEG0 | dest, entry

def make_structs():
    structs = ELFStructs(little_endian=False, elfclass=32)
    structs.create_basic_structs()
    structs.create_advanced_structs(e_type="ET_EXEC", e_machine="EM_MIPS", e_ident_osabi="ELFOSABI_SYSV")

    return structs

def build_ehdr(structs, entry, shoff):
    ident = Container(
        EI_MAG=[0x7F, ord("E"), ord("L"), ord("F")],
        EI_CLASS="ELFCLASS32",
        EI_DATA="ELFDATA2MSB",
        EI_VERSION="EV_CURRENT",
        EI_OSABI="ELFOSABI_SYSV",
        EI_ABIVERSION=0,
    )

    return structs.Elf_Ehdr.build(Container(
        e_ident=ident,
        e_type="ET_EXEC",
        e_machine="EM_MIPS",
        e_version="EV_CURRENT",
        e_entry=entry,
        e_phoff=EHDR_SIZE,
        e_shoff=shoff,
        e_flags=ELF_FLAGS,
        e_ehsize=EHDR_SIZE,
        e_phentsize=PHDR_SIZE,
        e_phnum=1,
        e_shentsize=SHDR_SIZE,
        e_shnum=3,
        e_shstrndx=2,
    ))

def build_phdr(structs, offset, vaddr, size):
    return structs.Elf_Phdr.build(Container(
        p_type="PT_LOAD",
        p_offset=offset,
        p_vaddr=vaddr,
        p_paddr=vaddr,
        p_filesz=size,
        p_memsz=size,
        p_flags=RWX_SEGMENT,
        p_align=WORD_SIZE,
    ))

def build_shdr(structs, name, kind, flags, addr, offset, size):
    return structs.Elf_Shdr.build(Container(
        sh_name=name,
        sh_type=kind,
        sh_flags=flags,
        sh_addr=addr,
        sh_offset=offset,
        sh_size=size,
        sh_link=0,
        sh_info=0,
        sh_addralign=WORD_SIZE if kind == "SHT_PROGBITS" else 1,
        sh_entsize=0,
    ))

def build_elf(image, vaddr, entry):
    structs = make_structs()

    image_offset = EHDR_SIZE + PHDR_SIZE
    shstrtab_offset = image_offset + len(image)
    shoff = (shstrtab_offset + len(SHSTRTAB) + WORD_SIZE - 1) & ~(WORD_SIZE - 1)
    padding = bytes(shoff - shstrtab_offset - len(SHSTRTAB))

    sections = [
        build_shdr(structs, 0, "SHT_NULL", 0, 0, 0, 0),
        build_shdr(structs, TEXT_NAME, "SHT_PROGBITS", RWX_SECTION, vaddr, image_offset, len(image)),
        build_shdr(structs, SHSTRTAB_NAME, "SHT_STRTAB", 0, 0, shstrtab_offset, len(SHSTRTAB)),
    ]

    return b"".join([
        build_ehdr(structs, entry, shoff),
        build_phdr(structs, image_offset, vaddr, len(image)),
        image,
        SHSTRTAB,
        padding,
        *sections,
    ])

def main():
    parser = argparse.ArgumentParser(description="Image into rom")
    parser.add_argument("--rom", required=True, help="linear program ROM")
    parser.add_argument("--out", required=True, help="ELF to write")
    args = parser.parse_args()

    with open(args.rom, "rb") as f:
        rom = f.read()

    try:
        image, vaddr, entry = read_load_info(rom)
    except ValueError as err:
        print(f"progrom2elf: {err}", file=sys.stderr)
        return 1

    with open(args.out, "wb") as f:
        f.write(build_elf(image, vaddr, entry))

    return 0

if __name__ == "__main__":
    sys.exit(main())
