#include <stdlib.h>
#include <string.h>

#include "cell_array.h"
#include "tools.h"

CellArray::CellArray(int width_, int height_)
    : array_width(width_), array_height(height_)
{
	sassert(array_width > 0, "invalid width: %d (expected > 0)", array_width);
	sassert(array_height > 0, "invalid height: %d (expected > 0)",
	        array_height);

	array = (Cell *)ntxm_ccalloc(sizeof(Cell), array_width * array_height);
}

CellArray::CellArray(Cell **ptn, int x1, int y1, int x2, int y2)
    : CellArray(x2 - x1 + 1, y2 - y1 + 1)
{
	if (valid())
		copy(ptn, x1, y1);
}

CellArray::~CellArray()
{
	if (array != NULL)
		ntxm_free(array);
}

bool CellArray::valid()
{
	return array != NULL;
}

int CellArray::width()
{
	return array_width;
}

int CellArray::height()
{
	return array_height;
}

Cell CellArray::at(int x, int y)
{
	return array[x * array_height + y];
}

Cell *CellArray::ptr(int x, int y)
{
	return &array[x * array_height + y];
}

void CellArray::copy(Cell **ptn, int x1, int y1)
{
	int x2 = x1 + array_width;

	Cell *dst = array;

	for (int x = x1; x < x2; x++, dst += array_height) {
		Cell *row = ptn[x] + y1;
		memcpy(dst, row, sizeof(Cell) * array_height);
	}
}

void CellArray::paste(Cell **ptn, int width, int height, int x1, int y1)
{
	int x2 = std::min(width, x1 + array_width);
	int y2 = std::min(height, y1 + array_height);

	Cell *src = array;

	for (int x = x1; x < x2; x++, src += array_height) {
		Cell *dst = ptn[x] + y1;
		memcpy(dst, src, sizeof(Cell) * (y2 - y1));
	}
}

void CellArray::paste(CellArray *cellarr, int x1, int y1)
{
	int x2 = std::min(cellarr->width(), x1 + array_width);
	int y2 = std::min(cellarr->height(), y1 + array_height);

	Cell *src = array;

	for (int x = x1; x < x2; x++, src += array_height) {
		for (int y = y1; y < y2; y++) {
			Cell *dst = cellarr->ptr(x, y);
			memcpy(dst, src + y, sizeof(Cell));
		}
	}
}

void CellArray::for_each(std::function<void(Cell *)> cell_fn)
{
	Cell *dst = array;
	for (int x = 0; x < array_width; x++)
		for (int y = 0; y < array_height; y++, dst++)
			cell_fn(dst);
}

CellArray *CellArray::clone()
{
	CellArray *new_i = new CellArray(array_width, array_height);
	if (new_i != NULL) {
		if (!new_i->valid()) {
			delete new_i;
			return NULL;
		}

		memcpy(new_i->array, array, sizeof(Cell) * array_width * array_height);
	}
	return new_i;
}
