// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2021-2023 agbabi contributors
//
// Support:
//    __ndsabi_rmemcpy, __ndsabi_rmemcpy1

#include "asminc.h"

#include "macros.inc"

    .syntax unified

    .arm


BEGIN_ASM_FUNC __ndsabi_rmemcpy

    @ >6-bytes is roughly the threshold when byte-by-byte copy is slower
    cmp     r2, #6
    ble     __ndsabi_rmemcpy1

    align_switch r0, r1, r3, __ndsabi_rmemcpy1, .Lcopy_halves

    @ Check if end needs word aligning
    add     r3, r0, r2
    joaobapt_test r3

    @ Copy byte tail to align
    submi   r2, r2, #1
    ldrbmi  r3, [r1, r2]
    strbmi  r3, [r0, r2]
    @ r2 is now half aligned

    @ Copy half tail to align
    subcs   r2, r2, #2
    ldrhcs  r3, [r1, r2]
    strhcs  r3, [r0, r2]
    @ r2 is now word aligned

    cmp     r2, #32
    blt     .Lcopy_words

    @ Word aligned, 32-byte copy
    push    {r0-r1, r4-r10}
    add     r0, r0, r2
    add     r1, r1, r2
.Lloop_32:
    subs    r2, r2, #32
    ldmdbge r1!, {r3-r10}
    stmdbge r0!, {r3-r10}
    bgt     .Lloop_32
    pop     {r0-r1, r4-r10}
    bxeq    lr

    @ < 32 bytes remaining to be copied
    add     r2, r2, #32

.Lcopy_words:
    subs    r2, r2, #4
    ldrge   r3, [r1, r2]
    strge   r3, [r0, r2]
    bgt     .Lcopy_words
    bxeq    lr

    @ Copy byte & half head
    joaobapt_test_into r3, r2
    @ Copy half
    addcs   r2, r2, #2
    ldrhcs  r3, [r1, r2]
    strhcs  r3, [r0, r2]
    @ Copy byte
    ldrbmi  r3, [r1]
    strbmi  r3, [r0]
    bx      lr

.Lcopy_halves:
    @ Copy byte tail to align
    add     r3, r0, r2
    tst     r3, #1
    subne   r2, r2, #1
    ldrbne  r3, [r1, r2]
    strbne  r3, [r0, r2]
    @ r2 is now half aligned

.Lloop_2:
    subs    r2, r2, #2
    ldrhge  r3, [r1, r2]
    strhge  r3, [r0, r2]
    bgt     .Lloop_2
    bxeq    lr

    @ Copy byte head
    ldrb    r3, [r1]
    strb    r3, [r0]
    bx      lr


BEGIN_ASM_FUNC_NO_SECTION __ndsabi_rmemcpy1

    subs    r2, r2, #1
    ldrbge  r3, [r1, r2]
    strbge  r3, [r0, r2]
    bgt     __ndsabi_rmemcpy1
    bx      lr
