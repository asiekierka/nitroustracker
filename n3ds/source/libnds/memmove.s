// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2021-2023 agbabi contributors
//
// ABI:
//    __aeabi_memmove, __aeabi_memmove4, __aeabi_memmove8
// Standard:
//    memmove

#include "asminc.h"

    .syntax unified

    .arm


BEGIN_ASM_FUNC __aeabi_memmove

    cmp     r0, r1
    bgt     __ndsabi_rmemcpy
    b       __aeabi_memcpy


BEGIN_ASM_FUNC __aeabi_memmove8
BEGIN_ASM_FUNC_NO_SECTION __aeabi_memmove4

    cmp     r0, r1
    bgt     __ndsabi_rmemcpy
    b       __aeabi_memcpy4


BEGIN_ASM_FUNC __ndsabi_memmove1

    cmp     r0, r1
    bgt     __ndsabi_rmemcpy1
    b       __ndsabi_memcpy1


BEGIN_ASM_FUNC memmove

    push    {r0, lr}
    bl      __aeabi_memmove
    pop     {r0, lr}
    bx      lr
