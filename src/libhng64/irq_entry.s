    .equ CAUSE_EXCCODE_MASK, 0x7C
    .equ ARG_SAVE_AREA, 32

    .equ SAVE_AT, ARG_SAVE_AREA
    .equ SAVE_V0, SAVE_AT + 8
    .equ SAVE_V1, SAVE_V0 + 8
    .equ SAVE_A0, SAVE_V1 + 8
    .equ SAVE_A1, SAVE_A0 + 8
    .equ SAVE_A2, SAVE_A1 + 8
    .equ SAVE_A3, SAVE_A2 + 8
    .equ SAVE_T0, SAVE_A3 + 8
    .equ SAVE_T1, SAVE_T0 + 8
    .equ SAVE_T2, SAVE_T1 + 8
    .equ SAVE_T3, SAVE_T2 + 8
    .equ SAVE_T4, SAVE_T3 + 8
    .equ SAVE_T5, SAVE_T4 + 8
    .equ SAVE_T6, SAVE_T5 + 8
    .equ SAVE_T7, SAVE_T6 + 8
    .equ SAVE_T8, SAVE_T7 + 8
    .equ SAVE_T9, SAVE_T8 + 8
    .equ SAVE_RA, SAVE_T9 + 8
    .equ SAVE_HI, SAVE_RA + 8
    .equ SAVE_LO, SAVE_HI + 8
    .equ SAVE_FCSR, SAVE_LO + 8
    .equ SAVE_FPR, SAVE_FCSR + 8
    .equ FRAME_SIZE, SAVE_FPR + 32 * 8 + 8

    .set noreorder
    .set noat

    .text
    .global hng_exception_entry
hng_exception_entry:
    mfc0    $k0, $13
    nop
    andi    $k0, $k0, CAUSE_EXCCODE_MASK
    bnez    $k0, exception_fatal
    nop

    addiu   $sp, $sp, -FRAME_SIZE

    sd      $at, SAVE_AT($sp)
    sd      $v0, SAVE_V0($sp)
    sd      $v1, SAVE_V1($sp)
    sd      $a0, SAVE_A0($sp)
    sd      $a1, SAVE_A1($sp)
    sd      $a2, SAVE_A2($sp)
    sd      $a3, SAVE_A3($sp)
    sd      $t0, SAVE_T0($sp)
    sd      $t1, SAVE_T1($sp)
    sd      $t2, SAVE_T2($sp)
    sd      $t3, SAVE_T3($sp)
    sd      $t4, SAVE_T4($sp)
    sd      $t5, SAVE_T5($sp)
    sd      $t6, SAVE_T6($sp)
    sd      $t7, SAVE_T7($sp)
    sd      $t8, SAVE_T8($sp)
    sd      $t9, SAVE_T9($sp)
    sd      $ra, SAVE_RA($sp)

    mfhi    $t0
    mflo    $t1
    sd      $t0, SAVE_HI($sp)
    sd      $t1, SAVE_LO($sp)

    # Handlers are plain hard float c so every fpr is fair game
    cfc1    $t0, $31
    sw      $t0, SAVE_FCSR($sp)
    sdc1    $f0, SAVE_FPR + 0 * 8($sp)
    sdc1    $f1, SAVE_FPR + 1 * 8($sp)
    sdc1    $f2, SAVE_FPR + 2 * 8($sp)
    sdc1    $f3, SAVE_FPR + 3 * 8($sp)
    sdc1    $f4, SAVE_FPR + 4 * 8($sp)
    sdc1    $f5, SAVE_FPR + 5 * 8($sp)
    sdc1    $f6, SAVE_FPR + 6 * 8($sp)
    sdc1    $f7, SAVE_FPR + 7 * 8($sp)
    sdc1    $f8, SAVE_FPR + 8 * 8($sp)
    sdc1    $f9, SAVE_FPR + 9 * 8($sp)
    sdc1    $f10, SAVE_FPR + 10 * 8($sp)
    sdc1    $f11, SAVE_FPR + 11 * 8($sp)
    sdc1    $f12, SAVE_FPR + 12 * 8($sp)
    sdc1    $f13, SAVE_FPR + 13 * 8($sp)
    sdc1    $f14, SAVE_FPR + 14 * 8($sp)
    sdc1    $f15, SAVE_FPR + 15 * 8($sp)
    sdc1    $f16, SAVE_FPR + 16 * 8($sp)
    sdc1    $f17, SAVE_FPR + 17 * 8($sp)
    sdc1    $f18, SAVE_FPR + 18 * 8($sp)
    sdc1    $f19, SAVE_FPR + 19 * 8($sp)
    sdc1    $f20, SAVE_FPR + 20 * 8($sp)
    sdc1    $f21, SAVE_FPR + 21 * 8($sp)
    sdc1    $f22, SAVE_FPR + 22 * 8($sp)
    sdc1    $f23, SAVE_FPR + 23 * 8($sp)
    sdc1    $f24, SAVE_FPR + 24 * 8($sp)
    sdc1    $f25, SAVE_FPR + 25 * 8($sp)
    sdc1    $f26, SAVE_FPR + 26 * 8($sp)
    sdc1    $f27, SAVE_FPR + 27 * 8($sp)
    sdc1    $f28, SAVE_FPR + 28 * 8($sp)
    sdc1    $f29, SAVE_FPR + 29 * 8($sp)
    sdc1    $f30, SAVE_FPR + 30 * 8($sp)
    sdc1    $f31, SAVE_FPR + 31 * 8($sp)

    jal     hng_irq_dispatch
    nop

    ldc1    $f0, SAVE_FPR + 0 * 8($sp)
    ldc1    $f1, SAVE_FPR + 1 * 8($sp)
    ldc1    $f2, SAVE_FPR + 2 * 8($sp)
    ldc1    $f3, SAVE_FPR + 3 * 8($sp)
    ldc1    $f4, SAVE_FPR + 4 * 8($sp)
    ldc1    $f5, SAVE_FPR + 5 * 8($sp)
    ldc1    $f6, SAVE_FPR + 6 * 8($sp)
    ldc1    $f7, SAVE_FPR + 7 * 8($sp)
    ldc1    $f8, SAVE_FPR + 8 * 8($sp)
    ldc1    $f9, SAVE_FPR + 9 * 8($sp)
    ldc1    $f10, SAVE_FPR + 10 * 8($sp)
    ldc1    $f11, SAVE_FPR + 11 * 8($sp)
    ldc1    $f12, SAVE_FPR + 12 * 8($sp)
    ldc1    $f13, SAVE_FPR + 13 * 8($sp)
    ldc1    $f14, SAVE_FPR + 14 * 8($sp)
    ldc1    $f15, SAVE_FPR + 15 * 8($sp)
    ldc1    $f16, SAVE_FPR + 16 * 8($sp)
    ldc1    $f17, SAVE_FPR + 17 * 8($sp)
    ldc1    $f18, SAVE_FPR + 18 * 8($sp)
    ldc1    $f19, SAVE_FPR + 19 * 8($sp)
    ldc1    $f20, SAVE_FPR + 20 * 8($sp)
    ldc1    $f21, SAVE_FPR + 21 * 8($sp)
    ldc1    $f22, SAVE_FPR + 22 * 8($sp)
    ldc1    $f23, SAVE_FPR + 23 * 8($sp)
    ldc1    $f24, SAVE_FPR + 24 * 8($sp)
    ldc1    $f25, SAVE_FPR + 25 * 8($sp)
    ldc1    $f26, SAVE_FPR + 26 * 8($sp)
    ldc1    $f27, SAVE_FPR + 27 * 8($sp)
    ldc1    $f28, SAVE_FPR + 28 * 8($sp)
    ldc1    $f29, SAVE_FPR + 29 * 8($sp)
    ldc1    $f30, SAVE_FPR + 30 * 8($sp)
    ldc1    $f31, SAVE_FPR + 31 * 8($sp)
    lw      $t0, SAVE_FCSR($sp)
    ctc1    $t0, $31

    # Hi and lo go back early so the mthi hazard is long gone by eret
    ld      $t0, SAVE_HI($sp)
    ld      $t1, SAVE_LO($sp)
    mthi    $t0
    mtlo    $t1

    ld      $at, SAVE_AT($sp)
    ld      $v0, SAVE_V0($sp)
    ld      $v1, SAVE_V1($sp)
    ld      $a0, SAVE_A0($sp)
    ld      $a1, SAVE_A1($sp)
    ld      $a2, SAVE_A2($sp)
    ld      $a3, SAVE_A3($sp)
    ld      $t0, SAVE_T0($sp)
    ld      $t1, SAVE_T1($sp)
    ld      $t2, SAVE_T2($sp)
    ld      $t3, SAVE_T3($sp)
    ld      $t4, SAVE_T4($sp)
    ld      $t5, SAVE_T5($sp)
    ld      $t6, SAVE_T6($sp)
    ld      $t7, SAVE_T7($sp)
    ld      $t8, SAVE_T8($sp)
    ld      $t9, SAVE_T9($sp)
    ld      $ra, SAVE_RA($sp)

    addiu   $sp, $sp, FRAME_SIZE

    # Another pending level just comes straight back in once exl drops
    eret
    nop

exception_fatal:
    b       exception_fatal
    nop
