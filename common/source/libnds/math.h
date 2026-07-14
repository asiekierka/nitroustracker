// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2005 Michael Noland (joat)
// Copyright (C) 2005 Jason Rogers (dovoto)
// Copyright (C) 2005 Dave Murphy (WinterMute)

/// @file nds/arm9/math.h
///
/// @brief hardware coprocessor math instructions.
///
/// @warning Only one type of sqrt and one type of division can be used
/// concurrently.

#ifndef LIBNDS_NDS_ARM9_MATH_H__
#define LIBNDS_NDS_ARM9_MATH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tobkit/platform.h"

// Fixed point conversion macros

#define inttof32(n) ((n) * (1 << 12))          ///< Convert int to f32
#define f32toint(n) ((n) / (1 << 12))          ///< Convert f32 to int
#define floattof32(n) ((int)((n) * (1 << 12))) ///< Convert float to f32
#define f32tofloat(n)                                                          \
	(((float)(n)) / (float)(1 << 12)) ///< Convert f32 to float

// Fixed Point versions

/// Fixed point divide
///
/// @param num
///     20.12 numerator.
/// @param den
///     20.12 denominator.
///
/// @return
///     Returns 20.12 result.
static inline int32_t divf32(int32_t num, int32_t den)
{
	return ((int64_t)((uint64_t)(int64_t)num << 12)) / den;
}

/// Fixed point multiply.
///
/// @param a
///     20.12 value.
/// @param b
///     20.12 value.
///
/// @return
///     Returns 20.12 result.
static inline int32_t mulf32(int32_t a, int32_t b)
{
	int64_t result = (int64_t)a * (int64_t)b;
	return (int32_t)(result >> 12);
}

#pragma GCC diagnostic push

// clang does not recognize -Wbuiltin-declaration-mismatch, ignore it
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
#endif

/// Fixed point sqrt.
///
/// @param a
///     20.12 positive value.
///
/// @return
///     20.12 result.
static inline uint32_t sqrtf32(uint32_t a)
{
	return (uint32_t)__builtin_sqrt((double)((uint64_t)a << 12));
}

// restore previous diagnostic settings (works with GCC and clang)
#pragma GCC diagnostic pop

/// Integer modulo.
///
/// @param num
///     Numerator.
/// @param den
///     Denominator.
///
/// @return
///     32 bit integer remainder.
static inline int32_t mod32(int32_t num, int32_t den)
{
	return num % den;
}

/// Integer 64 bit divide.
///
/// @param num
///     64 bit numerator.
/// @param den
///     32 bit denominator.
///
/// @return
///     32 bit integer result.
static inline int32_t div64(int64_t num, int32_t den)
{
	return num / den;
}

/// Integer 64 bit modulo.
///
/// @param num
///     64 bit numerator.
/// @param den
///     32 bit denominator.
///
/// @return
///     Returns 32 bit integer remainder.
static inline int32_t mod64(int64_t num, int32_t den)
{
	return num % den;
}

/// 32-bit integer sqrt.
///
/// @param a
///     32 bit positive integer value.
///
/// @return
///     32 bit integer result.
static inline uint32_t sqrt32(uint32_t a)
{
	return (uint32_t)__builtin_sqrt((double)a);
}

#ifdef __cplusplus
}
#endif

#endif // LIBNDS_NDS_ARM9_MATH_H__
