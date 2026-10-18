    .equ STATUS_INIT, 0x34000000
    .equ VECTOR_FIRST, 0x80000000
    .equ VECTOR_GENERAL, 0x80000180
    .equ VECTOR_STEP, 0x80
    .equ HIT_WB_INV_D, 0x15
    .equ HIT_INV_I, 0x10

    # o64 lets a callee spill its register args into the caller's frame
    .equ ARG_SAVE_AREA, 32
    .equ HEADER_SIZE, 0x80

    .set noreorder

    # Left zeroed for hng64kit-mkrom
    .section .header, "a"
    .space  HEADER_SIZE

    .text
    .global _start
_start:
    # Soft reset path skips the bios status write and leaves bev and erl set
    li      $t0, STATUS_INIT
    mtc0    $t0, $12

    # BIOS installs no handlers
    la      $t0, exception_stub
    lw      $t2, 0($t0)
    lw      $t3, 4($t0)
    li      $t0, VECTOR_FIRST
    li      $t1, VECTOR_GENERAL
install_vector:
    sw      $t2, 0($t0)
    sw      $t3, 4($t0)

    # kseg0 is cached so the cpu would fetch a stale line otherwise
    cache   HIT_WB_INV_D, 0($t0)
    cache   HIT_INV_I, 0($t0)

    addiu   $t0, $t0, VECTOR_STEP
    bne     $t0, $t1, install_vector
    nop

    # Interrupts land on the general vector so it jumps to the real entry
    la      $t1, general_vector
    lw      $t2, 0($t1)
    lw      $t3, 4($t1)
    sw      $t2, 0($t0)
    sw      $t3, 4($t0)
    cache   HIT_WB_INV_D, 0($t0)
    cache   HIT_INV_I, 0($t0)

    la      $sp, __stack_top - ARG_SAVE_AREA
    la      $gp, _gp

    la      $t0, __bss_start
    la      $t1, __bss_end
clear_bss:
    beq     $t0, $t1, run_main
    nop
    sw      $zero, 0($t0)
    b       clear_bss
    addiu   $t0, $t0, 4

run_main:
    jal     main
    nop

    # Main isn't supposed to return but park the cpu if it does
hang:
    b       hang
    nop

    # Get stuck wherever it gets copied
exception_stub:
    b       exception_stub
    nop

    # Copied to 0x80000180, j only keeps the top 4 bits and kseg0 shares them
general_vector:
    j       hng_exception_entry
    nop
