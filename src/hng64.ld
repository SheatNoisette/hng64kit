OUTPUT_ARCH(mips)
ENTRY(_start)

MEMORY
{
    ram (rwx) : ORIGIN = 0x80008000, LENGTH = 0x80400000 - 0x80008000
}

STACK_SIZE = 0x10000;

SECTIONS
{
    .text : {
        __image_start = .;

        /* BIOS looks for the header at cart rom offset 0 */
        KEEP(*(.header))
        *(.text .text.*)
    } > ram

    .rodata : {
        *(.rodata .rodata.*)
    } > ram

    .data : {
        *(.data .data.*)

        /* Newlib forces _impure_ptr into .sdata even at -G0 so gp must reach it */
        _gp = . + 0x7FF0;
        *(.sdata .sdata.*)

        /* Bios dma counts whole words */
        . = ALIGN(4);
        __image_end = .;
    } > ram

    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        *(.scommon)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > ram

    /DISCARD/ : {
        *(.MIPS.abiflags)
    }

    __stack_top = ORIGIN(ram) + LENGTH(ram);

    /* Newlib malloc grows up to the stack */
    __heap_start = __bss_end;
    __heap_end = __stack_top - STACK_SIZE;

    ASSERT(__heap_start < __heap_end, "no room left for the heap")
}
