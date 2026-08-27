.syntax unified
.text
.align 2

/*
 * u32 platinumMpuDisable(void)
 *
 * Called from Thumb cardengine.
 * Returns the original ARM9 CP15 control register.
 */
.thumb
.thumb_func
.global platinumMpuDisable
.type platinumMpuDisable, %function

platinumMpuDisable:
    ldr r3, =platinumMpuDisable_arm
    bx  r3

.align 2
.arm

platinumMpuDisable_arm:
    /*
     * CP15 c1: ARM946E-S control register.
     * Bit 0 = protection unit enable.
     */
    mrc p15, 0, r0, c1, c0, 0

    mov r1, r0
    bic r1, r1, #1

    mcr p15, 0, r1, c1, c0, 0

    nop
    nop
    nop

    /*
     * LR still contains a Thumb return address,
     * so BX switches us back to Thumb.
     */
    bx lr


/*
 * void platinumMpuRestore(u32 control)
 */
.thumb
.thumb_func
.global platinumMpuRestore
.type platinumMpuRestore, %function

platinumMpuRestore:
    ldr r3, =platinumMpuRestore_arm
    bx  r3

.align 2
.arm

platinumMpuRestore_arm:
    mcr p15, 0, r0, c1, c0, 0

    nop
    nop
    nop

    bx lr

.ltorg