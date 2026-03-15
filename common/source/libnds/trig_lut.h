// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2005 Michael Noland (joat)
// Copyright (C) 2005 Jason Rogers (dovoto)
// Copyright (C) 2005 Dave Murphy (WinterMute)

// Provides access to external precompiled trig look up tables

#ifndef LIBNDS_NDS_ARM9_TRIG_LUT_H__
#define LIBNDS_NDS_ARM9_TRIG_LUT_H__

#ifdef __cplusplus
extern "C" {
#endif

/// @file nds/arm9/trig_lut.h
///
/// @brief Fixed point trig functions. Angle can be in the range of -32768 to
/// 32767. There are 32768 degrees in the unit circle used by nds. To convert
/// between standard degrees (360 per circle):
///
/// ```
/// angle = degreesToAngle(angleInDegrees);
/// ```
///
/// or
///
/// ```
/// angle = angleInDegrees * 32768 / 360;
/// ```
///
/// This unit of measure is sometimes refered to as a binary radian (brad) or
/// binary degree. It allows for more precise representation of angle and faster
/// calculation as the DS has no floating point processor.

#include "tobkit/platform.h"

/// Number of degrees in a circle.
#define DEGREES_IN_CIRCLE (1 << 15)

/// Convert a fixed point number to an integer.
///
/// @param n
///     The number the number to convert.
/// @param bits
///     The number of bits used for the decimal part.
///
/// @return
///     The integer part.
#define fixedToInt(n, bits) ((int)((n) >> (bits)))

/// Converts an integer to a fixed point number.
///
/// @param n
///     The integer to convert.
/// @param bits
///     The number of bits used for the decimal part.
///
/// @return
///     The fixed point number.
#define intToFixed(n, bits) ((int)((n) << (bits)))

/// Converts a floating point number to a fixed point number.
///
/// @param n
///     The floating point number to convert.
/// @param bits
///     The number of bits used for the decimal part.
///
/// @return
/// The fixed point number.
#define floatToFixed(n, bits) ((int)((n) * (float)(1 << (bits))))

/// Converts a fixed point number to a floating point number.
///
/// @param n
///     The fixed point number to convert.
/// @param bits
///     The number of bits used for the decimal part.
///
/// @return
///     The floating point number.
#define fixedToFloat(n, bits) (((float)(n)) / (float)(1 << (bits)))

/// Removes the decimal part of a fixed point number.
///
/// @param n
///     The fixed point number.
/// @param bits
///     The number of bits used for the decimal part.
///
/// @return
///     A fixed point number with 0 as a decimal part.
#define floorFixed(n, bits) ((int)((n) & ~(((1 << (bits)) - 1))))

/// Convert an angle in 360 degree format to the format used by libnds.
#define degreesToAngle(degrees) ((degrees) * DEGREES_IN_CIRCLE / 360)

/// Converts an angle in the format used by libnds in the 360 degree format.
#define angleToDegrees(angle)   ((angle) * 360 / DEGREES_IN_CIRCLE)

/// Fixed point sine.
///
/// @param angle
///     Angle (-32768 to 32767).
///
/// @return
///     4.12 fixed point number with the range [-1, 1].
s16 sinLerp(s16 angle);

/// Fixed point cosine.
///
/// @param angle
///     Angle (-32768 to 32767).
///
/// @return
///     4.12 fixed point number with the range [-1, 1].
s16 cosLerp(s16 angle);

/// Fixed point tangent.
///
/// @param angle
///     Angle (-32768 to 32767).
///
/// @return
///     20.12 fixed point number with the range [-81.483, 524287.999].
s32 tanLerp(s16 angle);

/// Fixed point arcsin.
///
/// @param par
///     4.12 fixed point number with the range [-1, 1].
///
/// @return
///     Angle (-32768 to 32767).
s16 asinLerp(s16 par);

/// Fixed point arccos.
///
/// @param par
///     4.12 fixed point number with the range [-1, 1].
///
/// @return
///     Angle (-32768 to 32767).
s16 acosLerp(s16 par);

#ifdef __cplusplus
}
#endif

#endif // LIBNDS_NDS_ARM9_TRIG_LUT_H__
