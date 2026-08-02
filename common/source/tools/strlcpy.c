/* strlcpy(
   char *_PDCLIB_restrict _Dst,
   const char *_PDCLIB_restrict _Src,
   size_t _DstSize)

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include "tools.h"

#ifndef HAVE_STRLCPY

size_t strlcpy(
    char *restrict dst,
    const char *restrict src,
    size_t dstsize)
{
    size_t needed = 0;
    while(needed < dstsize && (dst[needed] = src[needed]))
        needed++;

    while(src[needed++]);

    if (needed > dstsize && dstsize)
      dst[dstsize - 1] = 0;

    return needed;
}

#endif
