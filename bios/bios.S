################################################################################
# FLAN BIOS FOR THE HNG64
# (c) SheatNoisette 2026 / MIT Licensed
################################################################################

    .equ SYSREG_BASE, 0xBF700000
    .equ SOFT_RESET_REG, 0xBF701114
    .equ DMA_BASE, 0xBF701200
    .equ CART_BASE, 0xA4000000
    .equ STACK_TOP, 0x80004000

    .equ SYSREG_14, 0x14
    .equ SYSREG_1C, 0x1C
    .equ SYSREG_1016, 0x1016
    .equ SYSREG_101F, 0x101F
    .equ SYSREG_1C_BUSY, 0x40

    .equ STATUS_SR, 0x00100000
    .equ STATUS_INIT, 0x34000000
    .equ CONFIG_K0_MASK, 0xFFFFFFF8
    .equ CONFIG_K0_UNCACHED, 0x2
    .equ CONFIG_K0_CACHED, 0x3
    .equ CONFIG_BE, 0x8000
    .equ FCSR_INIT, 0x01000000

    .equ KSEG0, 0x80000000
    .equ ICACHE_SIZE, 0x4000
    .equ DCACHE_SIZE, 0x2000
    .equ ICACHE_LINE, 32
    .equ DCACHE_LINE, 16
    .equ INDEX_INV_I, 0x00
    .equ INDEX_WB_INV_D, 0x01
    .equ INDEX_STORE_TAG_I, 0x08
    .equ INDEX_STORE_TAG_D, 0x09

    .equ CART_FLAGS, 0x24
    .equ CART_DMA_SRC, 0x50
    .equ CART_DMA_DST, 0x54
    .equ CART_DMA_WORDS, 0x58
    .equ CART_ENTRY, 0x5C
    .equ FLAG_FAST_BOOT, 0x8000
    .equ FLAG_NO_COPY, 0x2000

    .equ DMA_SRC, 0x04
    .equ DMA_DST, 0x14
    .equ DMA_COUNT, 0x24
    .equ DMA_MODE, 0x34
    .equ DMA_44, 0x44
    .equ DMA_CTRL, 0x4C
    .equ DMA_STATUS, 0x54
    .equ DMA_5C, 0x5C
    .equ DMA_MODE_COPY, 0x75
    .equ DMA_CTRL_START, 0x101
    .equ DMA_5C_KICK, 8
    .equ DMA_BUSY, 0x800
    .equ DMA_CHUNK_WORDS, 0x10000

    .set noreorder

    .text
    .global reset
reset:
    mfc0    $t0, $12
    li      $t1, STATUS_SR
    and     $t0, $t0, $t1
    beqz    $t0, cold_init
    nop

    li      $t0, SOFT_RESET_REG
    lw      $t1, 0($t0)
    andi    $t1, $t1, 1
    bnez    $t1, check_flags
    nop

cold_init:
    # Same as boot no idea what they are
    li      $t0, SYSREG_BASE
    li      $t1, 0x98
    sw      $t1, SYSREG_14($t0)
    li      $t1, SYSREG_1C_BUSY
    sw      $t1, SYSREG_1C($t0)
wait_sysreg:
    lw      $t1, SYSREG_1C($t0)
    andi    $t1, $t1, SYSREG_1C_BUSY
    bnez    $t1, wait_sysreg
    nop

    li      $t1, 0x94
    sb      $t1, SYSREG_101F($t0)
    li      $t1, 0x40
    sh      $t1, SYSREG_1016($t0)
    li      $t1, 0x28
    sw      $t1, SYSREG_1C($t0)

    # Keep kseg0 uncached until the caches hold some
    mfc0    $t0, $16
    li      $t1, CONFIG_K0_MASK
    and     $t0, $t0, $t1
    ori     $t0, $t0, CONFIG_K0_UNCACHED
    mtc0    $t0, $16
    nop
    nop
    nop
    nop

    # Zero tags mark every line invalid
    mtc0    $zero, $28
    mtc0    $zero, $29
    nop
    nop
    nop
    nop

    li      $t0, KSEG0
    li      $t1, KSEG0 + ICACHE_SIZE
init_icache:
    cache   INDEX_STORE_TAG_I, 0($t0)
    addiu   $t0, $t0, ICACHE_LINE
    bne     $t0, $t1, init_icache
    nop

    li      $t0, KSEG0
    li      $t1, KSEG0 + DCACHE_SIZE
init_dcache:
    cache   INDEX_STORE_TAG_D, 0($t0)
    addiu   $t0, $t0, DCACHE_LINE
    bne     $t0, $t1, init_dcache
    nop

    # Clearing bev and erl sends exceptions to ram
    li      $t0, STATUS_INIT
    mtc0    $t0, $12
    nop
    nop
    nop
    nop

    mfc0    $t0, $16
    ori     $t0, $t0, CONFIG_BE
    mtc0    $t0, $16
    mtc0    $zero, $18
    mtc0    $zero, $19
    nop
    nop
    nop
    nop

    li      $sp, STACK_TOP

check_flags:
    # Games may count on denormals flushing ?
    li      $t0, FCSR_INIT
    ctc1    $t0, $31

    li      $s0, CART_BASE
    lhu     $s1, CART_FLAGS($s0)
    andi    $t0, $s1, FLAG_FAST_BOOT
    beqz    $t0, hang
    nop

    andi    $t0, $s1, FLAG_NO_COPY
    bnez    $t0, jump_game
    nop

    # Soft reset leaves the caches alive so dirty lines could land on the copy
    # and crap out everything
    li      $t0, KSEG0
    li      $t1, KSEG0 + DCACHE_SIZE
flush_dcache:
    cache   INDEX_WB_INV_D, 0($t0)
    addiu   $t0, $t0, DCACHE_LINE
    bne     $t0, $t1, flush_dcache
    nop

    li      $t0, KSEG0
    li      $t1, KSEG0 + ICACHE_SIZE
flush_icache:
    cache   INDEX_INV_I, 0($t0)
    addiu   $t0, $t0, ICACHE_LINE
    bne     $t0, $t1, flush_icache
    nop

    # Official copy turns the cache on, guess  so the game gets the same
    mfc0    $t0, $16
    li      $t1, CONFIG_K0_MASK
    and     $t0, $t0, $t1
    ori     $t0, $t0, CONFIG_K0_CACHED
    mtc0    $t0, $16
    nop
    nop
    nop
    nop

    li      $t0, DMA_BASE
    lw      $t1, CART_DMA_SRC($s0)
    lw      $t2, CART_DMA_DST($s0)
    lw      $t3, CART_DMA_WORDS($s0)
    li      $t4, DMA_CHUNK_WORDS

    sw      $zero, DMA_44($t0)
    li      $t5, DMA_MODE_COPY
    sw      $t5, DMA_MODE($t0)
    sw      $t1, DMA_SRC($t0)
    sw      $t2, DMA_DST($t0)

    # Count Reg is too small for big games and it keeps advancing
    # the addresses
dma_chunk:
    move    $t6, $t3
    sltu    $t5, $t4, $t3
    beqz    $t5, dma_start
    nop
    move    $t6, $t4

dma_start:
    addiu   $t5, $t6, -1
    sw      $t5, DMA_COUNT($t0)
    li      $t5, DMA_CTRL_START
    sw      $t5, DMA_CTRL($t0)
    li      $t5, DMA_5C_KICK
    sw      $t5, DMA_5C($t0)
dma_wait:
    lw      $t5, DMA_STATUS($t0)
    andi    $t5, $t5, DMA_BUSY
    bnez    $t5, dma_wait
    nop

    subu    $t3, $t3, $t6
    bnez    $t3, dma_chunk
    nop

    sw      $zero, DMA_CTRL($t0)

jump_game:
    lw      $t0, CART_ENTRY($s0)
    jr      $t0
    nop

hang:
    b       hang
    nop
