// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2017-2018 Dave Murphy (WinterMute)
// Copyright (C) 2025 Antonio Niño Díaz

#ifndef LIBNDS_ASMINC_H__
#define LIBNDS_ASMINC_H__

#if !__ASSEMBLER__
# error "This header file is only for use in assembly files!"
#endif

.macro BEGIN_ASM_FUNC name section=text arch=unset
    .section .\section\().\name\(), "ax", %progbits
    .global \name
    .type \name, %function

    .if \arch == thumb
        .balign         2
        .thumb
    .elseif \arch == arm
        .balign         4
        .arm
    .elseif \arch == unset
        // If the architecture it isn't specified, don't set it in this macro.
        // Set the aligment to the one that will work in all cases.
        .balign         4
    .else
        .error "Unknown architecture"
    .endif
\name:
.endm

// Remember to use this macro if one function falls through the following one.
// If not, the linker may choose to remove the second one if it isn't referenced
// anywhere else.
.macro BEGIN_ASM_FUNC_NO_SECTION name arch=unset
    .global \name
    .type \name, %function

    .if \arch == thumb
        .balign         2
        .thumb
    .elseif \arch == arm
        .balign         4
        .arm
    .elseif \arch == unset
        // If the architecture it isn't specified, don't set it in this macro.
        // Set the aligment to the one that will work in all cases.
        .balign         4
    .else
        .error "Unknown architecture"
    .endif
\name:
.endm

#define ICACHE_SIZE     0x2000
#define DCACHE_SIZE     0x1000
#define CACHE_LINE_SIZE 32

#endif // LIBNDS_ASMINC_H__
