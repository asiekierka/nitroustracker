#ifndef ACTION_H
#define ACTION_H

#include <functional>
#include "state.h"
#include "cell_array.h"
#include "ntxm/song.h"

/**
 * apply() and revert() must never be called multiple times in a row!
 * 
 * This is used to conserve memory (re-using the same fields of data).
 * 
 * apply() -> apply() - not ok
 * apply() -> revert() -> apply() - ok
 */
class Action {
    public:
        virtual ~Action() = 0;
        virtual bool valid() { return true; };
        virtual bool apply(Song *song) = 0;
        virtual bool revert(Song *song) = 0;
};

class PatternLocatedAction: public Action {
    public:
        PatternLocatedAction(State *state, u8 channel, u16 row);
        PatternLocatedAction(State *state): PatternLocatedAction(state, state->channel, state->getCursorRow()) {};
        virtual ~PatternLocatedAction() = 0;

    protected:
		u8 potpos;
		u16 row;
		u8 channel;
        bool validateCoords(Song *song, int width, int height);
        bool validateCoords(Song *song) { return validateCoords(song, 1, 1); };
};

class SingleCellSetAction: public PatternLocatedAction {
    public:
        SingleCellSetAction(State *state, u8 channel, u16 row, Cell targetCell);
        bool apply(Song *song);
        bool revert(Song *song);

    protected:
        Cell cell;
};

class MultipleCellSetAction: public PatternLocatedAction {
    public:
        MultipleCellSetAction(State *state, u8 channel, u16 row, CellArray *array, bool clone);
        ~MultipleCellSetAction();
        bool apply(Song *song);
        bool revert(Song *song);

    protected:
        CellArray *array;
        std::function<Cell(Cell)> applyFunc;
};

MultipleCellSetAction *newCellClearAction(State *state, Song *song, u16 x1, u16 y1, u16 x2, u16 y2);

class CellInsertAction: public PatternLocatedAction {
    public:
        CellInsertAction(State *state, u8 channel, u16 row, u16 width, u16 height);
        ~CellInsertAction();
        bool apply(Song *song);
        bool revert(Song *song);

    protected:
        CellArray *array;
};

class CellDeleteAction: public PatternLocatedAction {
    public:
        CellDeleteAction(State *state, u8 channel, u16 row, u16 width, u16 height);
        ~CellDeleteAction();
        bool apply(Song *song);
        bool revert(Song *song);

    protected:
        CellArray *array;
};

class ActionBuffer {
	public:
        ActionBuffer(int size);

		~ActionBuffer();
		
        bool valid();

        void clear();

        bool add(Song *song, Action *action);

        bool undo(Song *song);
        bool redo(Song *song);

        bool can_undo();
        bool can_redo();
        int queue_length();
        int size();

        void register_change_callback(std::function<void(void)> func);
	private:
        void clear_at(int i);

        std::function<void(void)> on_change;
        int action_size;
        int a_head; // first entry in circular buffer
        int a_tail; // last entry in circular buffer
        int a_pos; // current entry in circular buffer
		Action **actions;
};

#endif
