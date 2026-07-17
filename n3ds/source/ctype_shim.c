// This lightweight shim shaves about 15 KiB off the .3dsx build size.

#include <stddef.h>
#include <wctype.h>

int iswspace(wint_t ch)
{
	return ch == 0x20 || (ch >= 0x09 && ch <= 0x0d);
}

int isdigit(int ch)
{
	return ch >= '0' && ch <= '9';
}
