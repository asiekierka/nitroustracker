#include <stdlib.h>
#include <string.h>

#include "action.h"
#include "tools.h"

// actions

Action::~Action() { }
PatternLocatedAction::~PatternLocatedAction() { }


PatternLocatedAction::PatternLocatedAction(State *state, u8 channel_, u16 row_)
    :potpos(state->potpos), row(row_), channel(channel_) { }

bool PatternLocatedAction::validateCoords(Song *song, int width, int height)
{
    if ((channel+width-1) >= song->getChannels())
        return false;
    if (potpos >= song->getPotLength())
        return false;
    u8 potEntry = song->getPotEntry(potpos);
    if ((row+height-1) >= song->getPatternLength(potEntry))
        return false;
    return true;
}


SingleCellSetAction::SingleCellSetAction(State *state, u8 channel, u16 row, Cell cell_)
    :PatternLocatedAction(state, channel, row), cell(cell_) { }

bool SingleCellSetAction::apply(Song *song)
{
    if (!validateCoords(song)) return false;

    Cell *ptr = &song->getPattern(song->getPotEntry(potpos))[channel][row];

    Cell oldCell = *ptr;
    *ptr = cell;
    cell = oldCell;

    return true;
}

bool SingleCellSetAction::revert(Song *song)
{
    return apply(song);
}


MultipleCellSetAction::MultipleCellSetAction(State *state, u8 channel, u16 row, CellArray *array_, bool clone)
    :PatternLocatedAction(state, channel, row)
{
    array = clone ? array_->clone() : array_;
    if (clone && !array->valid())
    {
        delete(array);
        array = NULL;
    }
}

MultipleCellSetAction::~MultipleCellSetAction()
{
    if (array != NULL) delete(array);
}

bool MultipleCellSetAction::apply(Song *song)
{
    if ((array == NULL) || !validateCoords(song, array->width(), array->height())) return false;

    for (int x = 0; x < array->width(); x++)
        for (int y = 0; y < array->height(); y++)
        { 
            Cell *ptr = &song->getPattern(song->getPotEntry(potpos))[channel + x][row + y];

            Cell oldCell = *ptr;
            *ptr = array->at(x, y);
            *array->ptr(x, y) = oldCell;
        }

    return true;
}

bool MultipleCellSetAction::revert(Song *song)
{
    return apply(song);
}


MultipleCellSetAction *newCellClearAction(State *state, Song *song, u16 x1, u16 y1, u16 x2, u16 y2)
{
    CellArray *clear = new CellArray(x2 - x1 + 1, y2 - y1 + 1);
    if (clear != NULL && clear->valid())
    {
        clear->for_each([song](Cell *c) { song->clearCell(c); });
        return new MultipleCellSetAction(state, x1, y1, clear, false);
    }
    return NULL;
}


CellInsertAction::CellInsertAction(State *state, u8 channel, u16 row, u16 width_, u16 height_)
    :PatternLocatedAction(state, channel, row)
{
    array = new CellArray(width_, height_);
    if (!array->valid())
    {
        delete(array);
        array = NULL;
    }
}

CellInsertAction::~CellInsertAction()
{
    if (array != NULL) delete(array);
}

bool CellInsertAction::apply(Song *song)
{
    if ((array == NULL) || !validateCoords(song, array->width(), array->height())) return false;

	Cell **ptn = song->getPattern(song->getPotEntry(potpos));
	u16 ptn_len = song->getPatternLength(song->getPotEntry(potpos));
    u16 sel_x1 = channel;
    u16 sel_x2 = channel + array->width() - 1;

    // backup
    array->copy(ptn, channel, ptn_len - 1 - array->height());

    // insert
	for(s32 rowy=ptn_len - 1 - array->height(); rowy >= row; --rowy)
		for (s32 chn=sel_x1; chn <= sel_x2; ++chn)
			ptn[chn][rowy + array->height()] = ptn[chn][rowy];

	for(s32 rowy=0; rowy < array->height(); ++rowy)
		for (s32 chn=sel_x1; chn <= sel_x2; ++chn)
			song->clearCell(&ptn[chn][row + rowy]);

    return true;
}

bool CellInsertAction::revert(Song *song)
{
    if ((array == NULL) || !validateCoords(song, array->width(), array->height())) return false;
     
	Cell **ptn = song->getPattern(song->getPotEntry(potpos));
	u16 ptn_len = song->getPatternLength(song->getPotEntry(potpos));
    u16 sel_x1 = channel;
    u16 sel_x2 = channel + array->width() - 1;
    
    // delete
	for(s32 rowy = row + array->height(); rowy < ptn_len; rowy++)
		for (s32 chn=sel_x1; chn <= sel_x2; ++chn)
			ptn[chn][rowy - array->height()] = ptn[chn][rowy];
        
    // restore
    array->paste(ptn, song->getChannels(), ptn_len, channel, ptn_len - 1 - array->height());

    return true;
}


CellDeleteAction::CellDeleteAction(State *state, u8 channel, u16 row, u16 width_, u16 height_)
    :PatternLocatedAction(state, channel, row)
{
    array = new CellArray(width_, height_);
    if (!array->valid())
    {
        delete(array);
        array = NULL;
    }
}

CellDeleteAction::~CellDeleteAction()
{
    if (array != NULL) delete(array);
}

bool CellDeleteAction::apply(Song *song)
{
    if ((array == NULL) || !validateCoords(song, array->width(), array->height())) return false;

	Cell **ptn = song->getPattern(song->getPotEntry(potpos));
	u16 ptn_len = song->getPatternLength(song->getPotEntry(potpos));
    u16 sel_x1 = channel;
    u16 sel_x2 = channel + array->width() - 1;

    // backup
    array->copy(ptn, channel, row);

    // delete
	for(s32 rowy = row + array->height(); rowy < ptn_len; rowy++)
		for (s32 chn=sel_x1; chn <= sel_x2; ++chn)
			ptn[chn][rowy - array->height()] = ptn[chn][rowy];

	for(s32 rowy = ptn_len - array->height(); rowy < ptn_len; ++rowy)
		for (s32 chn=sel_x1; chn <= sel_x2; ++chn)
			song->clearCell(&ptn[chn][rowy]);

    return true;
}

bool CellDeleteAction::revert(Song *song)
{
    if ((array == NULL) || !validateCoords(song, array->width(), array->height())) return false;
     
	Cell **ptn = song->getPattern(song->getPotEntry(potpos));
	u16 ptn_len = song->getPatternLength(song->getPotEntry(potpos));
    u16 sel_x1 = channel;
    u16 sel_x2 = channel + array->width() - 1;
    
    // insert
	for(s32 rowy=ptn_len - 1 - array->height(); rowy >= row; --rowy)
		for (s32 chn=sel_x1; chn <= sel_x2; ++chn)
			ptn[chn][rowy + array->height()] = ptn[chn][rowy];
        
    // restore
    array->paste(ptn, song->getChannels(), ptn_len, channel, row);

    return true;
}

// buffer

#define ACTION_BUFFER_INC(i) (((i) + 1) % action_size)
#define ACTION_BUFFER_DEC(i) (((i) == 0) ? (action_size - 1) : ((i) - 1))

void ActionBuffer::clear_at(int i)
{
    if (actions[i] != NULL)
    {
        delete actions[i];
        actions[i] = NULL;
    }
}

ActionBuffer::ActionBuffer(int size)
    :on_change([]{}), action_size(size)
{
    actions = (Action**) ntxm_ccalloc(sizeof(Action*), action_size);
    clear();
}

ActionBuffer::~ActionBuffer()
{
    if (actions != NULL) ntxm_free(actions);
}

bool ActionBuffer::valid()
{
    return actions != NULL;
}

void ActionBuffer::clear()
{
    for (int i = 0; i < action_size; i++)
        clear_at(i);

    a_head = 0;
    a_tail = 0;
    a_pos = 0;

    on_change();
}

bool ActionBuffer::add(Song *song, Action *action)
{
    if (action == NULL) return false;

    bool res = action->apply(song);

    if (res)
    {
        // delete everything between a_pos and a_tail
        for (int i = a_pos; i != a_tail; i = ACTION_BUFFER_INC(i))
            clear_at(i);
        a_tail = a_pos;
        // check for full buffer
        if (a_head == a_tail && actions[a_tail] != NULL)
        {
            clear_at(a_head);
            a_head = ACTION_BUFFER_INC(a_head);
        }
        // apply action
        actions[a_tail] = action;
        a_tail = ACTION_BUFFER_INC(a_tail);
        a_pos = a_tail;
        on_change();
        debugprintf("undo push => %d\n", queue_length());
        ntxm_flush_dcache();
        return true;
    }
    else
    {
        debugprintf("undo buffer trashed!\n");
        clear();
        return false;
    }
    
}

bool ActionBuffer::undo(Song *song)
{
    if (!can_undo()) return false;

    int undo_pos = ACTION_BUFFER_DEC(a_pos);
    sassert(actions[undo_pos] != NULL, "action is null! (%d %d)", a_head, a_tail);

    bool res = actions[undo_pos]->revert(song);
    if (res)
    {
        // undo successful, descend
        a_pos = undo_pos;
        on_change();
        return true;
    }
    else
    {
        // undo failed, clear all
        debugprintf("undo buffer trashed!\n");
        clear();
        return false;
    }
}

bool ActionBuffer::redo(Song *song)
{
    if (!can_redo()) return false;

    int redo_pos = a_pos;
    sassert(actions[redo_pos] != NULL, "action is null! (%d %d)", a_head, a_tail);

    bool res = actions[redo_pos]->apply(song);
    if (res)
    {
        // redo successful, ascend
        a_pos = ACTION_BUFFER_INC(a_pos);
        on_change();
        return true;
    }
    else
    {
        // undo failed, clear all
        debugprintf("redo buffer trashed!\n");
        clear();
        return false;
    }
}

bool ActionBuffer::can_undo()
{
    return queue_length() > 0 && a_pos != a_head;
}

bool ActionBuffer::can_redo()
{
    return queue_length() > 0 && a_pos != a_tail;
}

int ActionBuffer::queue_length()
{
    int x = a_tail - a_head;
    return x < 0 ? (action_size + x) : x;
}

int ActionBuffer::size()
{
    return action_size;
}

void ActionBuffer::register_change_callback(std::function<void(void)> func) {
    on_change = func;
}
