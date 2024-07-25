// PsProject.cpp

/**
 * This file is part of Patternshop Project.
 *
 * Patternshop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Patternshop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Patternshop.  If not, see <http://www.gnu.org/licenses/>
*/

#include <stdio.h>
#include "PsProjectController.h"
#include "PsController.h"
#include "PsAction.h"
#include "PsMatrix.h"
#include "PsPattern.h"
#include "PsSoftRender.h"
#include "PsWinPropertiesCx.h"


/*
** Resets a project_controller by clearing everything it contains. Eventually, this function
** could become unnecessary and simply be copied into the destructor of "PsProject".
*/
void PsProjectController::LogFlush()
{
    LogList::iterator t;
    MatrixList::iterator i;

    for (t = this->log_redo.begin(); t != this->log_redo.end(); ++t)
        delete* t;

    for (t = this->log_undo.begin(); t != this->log_undo.end(); ++t)
        delete* t;

    for (i = this->matrices.begin(); i != this->matrices.end(); ++i)
        delete* i;

    this->matrices.clear();

    if (this->pattern)
    {
        delete this->pattern;
        this->pattern = 0;
    }

    this->matrix = 0;
    this->image = 0;
    this->shape = 0;
}

/*
** Inserts a new element into the log
*/
void PsProjectController::LogAdd(PsAction* log)
{
    LogList::iterator t;

    for (t = this->log_redo.begin(); t != this->log_redo.end(); ++t)
        delete* t;

    this->log_redo.clear();

    if (log)
        this->log_undo.push_back(log);

    while (this->log_undo.size() > LOG_SIZE)
    {
        delete this->log_undo.front();
        this->log_undo.pop_front();
    }

    this->log_insert = false;
    this->bNeedSave = true;
}

/*
** Checks if an action can be redone
*/
bool PsProjectController::LogCanRedo() const
{
    return !this->log_redo.empty();
}

/*
** Checks if an action can be undone
*/
bool PsProjectController::LogCanUndo() const
{
    return !this->log_undo.empty();
}

/*
** Initializes a new action (marks the end of the previous action)
*/
void PsProjectController::LogInit()
{
    this->log_insert = true;
}

/*
** Returns the number of elements in the undo list
*/
int PsProjectController::LogUndoCount()
{
    return this->log_undo.size();
}

/*
** Returns the name of the last "redoable" action recorded
*/
const char* PsProjectController::LogRedoLastName() const
{
    return(*this->log_redo.rbegin())->Name();
}

/*
** Returns the name of the last "undoable" action recorded
*/
const char* PsProjectController::LogUndoLastName() const
{
    return(*this->log_undo.rbegin())->Name();
}

/*
** Checks if the action is new and should be recorded (and resets the flag if necessary)
*/
bool PsProjectController::LogMustAdd() const
{
    return this->log_insert;
}

/*
** Reproduces the next action
*/
void PsProjectController::LogRedo()
{
    PsAction* redo;
    PsAction* undo;

    if (!this->log_redo.empty())
    {
        redo = *this->log_redo.rbegin();
        this->log_redo.pop_back();
        undo = redo->Execute();

        if (undo)
        {
            this->log_undo.push_back(undo);
            this->bNeedSave = true;
        }

        while (this->log_undo.size() > LOG_SIZE)
        {
            delete this->log_undo.front();
            this->log_undo.pop_front();
        }

        delete redo;
    }
}

/*
** Undoes the last action
*/
void PsProjectController::LogUndo()
{
    PsAction* redo;
    PsAction* undo;

    if (!this->log_undo.empty())
    {
        undo = *this->log_undo.rbegin();
        this->log_undo.pop_back();
        redo = undo->Execute();

        this->bNeedSave = true;

        if (redo)
            this->log_redo.push_back(redo);

        while (this->log_redo.size() > LOG_SIZE)
        {
            delete this->log_redo.front();
            this->log_redo.pop_front();
        }

        delete undo;
    }
}

/*
** Changes the color of the active matrix, if there is one.
*/
ErrID PsProjectController::MatrixColor()
{
    if (!this->matrix)
        return ERROR_MATRIX_SELECT;

    this->matrix->DoChangeColor();

    return ERROR_NONE;
}

/*
** Resets the transformations of the active matrix, if there is one.
*/
ErrID PsProjectController::MatrixReset()
{
    if (!this->matrix)
        return ERROR_MATRIX_SELECT;

    this->LogAdd(new LogResize(*this, this->matrix, this->matrix->x, this->matrix->y, this->matrix->w, this->matrix->h, false));
    this->LogAdd(new LogRotate(*this, this->matrix, this->matrix->r, false));
    this->LogAdd(new LogTorsio(*this, this->matrix, this->matrix->i, this->matrix->j));

    this->matrix->DoResetAll();

    return ERROR_NONE;
}
