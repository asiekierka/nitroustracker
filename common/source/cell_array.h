#ifndef CELL_ARRAY_H
#define CELL_ARRAY_H

#include <functional>
#include "ntxm/song.h"

class CellArray {
	public:
		CellArray(int width, int height);
        CellArray(Cell **ptn, int x1, int y1, int x2, int y2);

		~CellArray();
		
        bool valid();

        int width();
        int height();

        Cell at(int x, int y);
        Cell *ptr(int x, int y);
        void for_each(std::function<void(Cell*)> cell);

        void copy(Cell **ptn, int x1, int y1);
        void paste(Cell **ptn, int width, int height, int x1, int y1);
        void paste(CellArray *cellarr, int x1, int y1);

        CellArray *clone();
		
	private:
		int array_width;
        int array_height;

        Cell *array;
};

#endif
