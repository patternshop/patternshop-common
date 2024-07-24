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
#include "PsProject.h"
#include "PsController.h"
#include "PsAction.h"
#include "PsMatrix.h"
#include "PsPattern.h"
#include "PsSoftRender.h"
#include "PsWinPropertiesCx.h"

/**
 * Loads the project from a file. The constant macro "PROJECT_VERSION" is used to check the version
 * of the save and thus serves as a magic number. The rest should be quite explicit; we load all
 * the fields of the project using their FileLoad methods.
 */
ErrID PsProject::FileLoad(const char* path)
{
    PsMatrix* matrix;
    FILE* file;
    ErrID err;
    size_t n;
    int x;
    int y;

    if (!(file = fopen(path, "rb")))
        return ERROR_FILE_ACCESS;

    PsController::Instance().SetProgress(-1);

    this->LogFlush();

    fread(&n, sizeof(size_t), 1, file);

    if (n != ACCEPTED_ENDIAN_FILE)
    {
        PsController::Instance().SetProgress(-2);
        return ERROR_FILE_VERSION;
    }

    fread(&n, sizeof(size_t), 1, file);

    if (n != PROJECT_VERSION && n != PROJECT_VERSION_OLD_BETA) // FIXME
    {
        PsController::Instance().SetProgress(-2);
        return ERROR_FILE_VERSION;
    }

    fread(&x, sizeof(x), 1, file);
    fread(&y, sizeof(y), 1, file);
    this->renderer.SetDocSize(x, y);

    PsController::Instance().SetProgress(10);

    if (n != PROJECT_VERSION_OLD_BETA) // FIXME
    {
        if (fread(&n, sizeof(n), 1, file) != 1)
        {
            PsController::Instance().SetProgress(-2);
            return ERROR_FILE_READ;
        }
        while (n--)
        {
            this->images.push_back((this->image = new PsImage(NULL)));

            if ((err = this->image->FileLoad(file)) != ERROR_NONE)
            {
                PsController::Instance().SetProgress(-2);
                return err;
            }
        }
    }

    fread(&n, sizeof(n), 1, file);

    for (uint32 i = 0; i < n; ++i)
    {
        PsController::Instance().SetProgress(10 + 90 * i / n);

        this->matrices.push_back((matrix = new PsMatrix()));

        if ((err = matrix->FileLoad(file)) != ERROR_NONE)
        {
            PsController::Instance().SetProgress(-2);
            return err;
        }
    }

    bool pattern_exist = false;
    fread(&pattern_exist, sizeof(pattern_exist), 1, file);
    if (pattern_exist)
    {
        this->pattern = new PsPattern();
        if ((err = this->pattern->FileLoad(file)) != ERROR_NONE)
        {
            PsController::Instance().SetProgress(-2);
            return err;
        }
        this->pattern->UpdateScale(this->GetWidth(), this->GetHeight());
    }

    fread(&this->bHideColor, sizeof(this->bHideColor), 1, file);
    fread(&this->iColor, sizeof(this->iColor), 1, file);

    fclose(file);

    this->center = true;

    PsController::Instance().SetProgress(-2);

    return ERROR_NONE;
}

/*
** Saves a project to a file (see FileLoad).
*/
ErrID PsProject::FileSave(const char* path)
{
    FILE* file;
    ErrID err;
    size_t n;
    int x;
    int y;

    if (!(file = fopen(path, "wb")))
        return ERROR_FILE_ACCESS;

    n = ACCEPTED_ENDIAN_FILE;
    fwrite(&n, sizeof(size_t), 1, file);

    n = PROJECT_VERSION;
    fwrite(&n, sizeof(size_t), 1, file);

    this->renderer.GetDocSize(x, y);
    fwrite(&x, sizeof(int), 1, file);
    fwrite(&y, sizeof(int), 1, file);

    n = this->images.size();

    if (fwrite(&n, sizeof(size_t), 1, file) != 1)
        return ERROR_FILE_WRITE;

    for (ImageList::const_iterator i = this->images.begin(); i != this->images.end(); ++i)
        if ((err = (*i)->FileSave(file)) != ERROR_NONE)
            return err;

    n = this->matrices.size();
    fwrite(&n, sizeof(size_t), 1, file);

    for (MatrixList::const_iterator i = this->matrices.begin(); i != this->matrices.end(); ++i)
        if ((err = (*i)->FileSave(file)) != ERROR_NONE)
        {
            fclose(file);
            return err;
        }

    bool pattern_exist = false;
    if (this->pattern) pattern_exist = true;
    fwrite(&pattern_exist, sizeof(pattern_exist), 1, file);
    if (pattern_exist)
    {
        if ((err = this->pattern->FileSave(file)) != ERROR_NONE)
        {
            fclose(file);
            return err;
        }
    }

    fwrite(&this->bHideColor, sizeof(this->bHideColor), 1, file);
    fwrite(&this->iColor, sizeof(this->iColor), 1, file);

    fclose(file);

    this->bNeedSave = false;

    return ERROR_NONE;
}

/*
** Resets a project by clearing everything it contains. Eventually, this function
** could become unnecessary and simply be copied into the destructor of "PsProject".
*/
void PsProject::LogFlush()
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
void PsProject::LogAdd(PsAction* log)
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
bool PsProject::LogCanRedo() const
{
    return !this->log_redo.empty();
}

/*
** Checks if an action can be undone
*/
bool PsProject::LogCanUndo() const
{
    return !this->log_undo.empty();
}

/*
** Initializes a new action (marks the end of the previous action)
*/
void PsProject::LogInit()
{
    this->log_insert = true;
}

/*
** Returns the number of elements in the undo list
*/
int PsProject::LogUndoCount()
{
    return this->log_undo.size();
}

/*
** Returns the name of the last "redoable" action recorded
*/
const char* PsProject::LogRedoLastName() const
{
    return(*this->log_redo.rbegin())->Name();
}

/*
** Returns the name of the last "undoable" action recorded
*/
const char* PsProject::LogUndoLastName() const
{
    return(*this->log_undo.rbegin())->Name();
}

/*
** Checks if the action is new and should be recorded (and resets the flag if necessary)
*/
bool PsProject::LogMustAdd() const
{
    return this->log_insert;
}

/*
** Reproduces the next action
*/
void PsProject::LogRedo()
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
void PsProject::LogUndo()
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
ErrID PsProject::MatrixColor()
{
    if (!this->matrix)
        return ERROR_MATRIX_SELECT;

    this->matrix->DoChangeColor();

    return ERROR_NONE;
}

/*
** Resets the transformations of the active matrix, if there is one.
*/
ErrID PsProject::MatrixReset()
{
    if (!this->matrix)
        return ERROR_MATRIX_SELECT;

    this->LogAdd(new LogResize(*this, this->matrix, this->matrix->x, this->matrix->y, this->matrix->w, this->matrix->h, false));
    this->LogAdd(new LogRotate(*this, this->matrix, this->matrix->r, false));
    this->LogAdd(new LogTorsio(*this, this->matrix, this->matrix->i, this->matrix->j));

    this->matrix->DoResetAll();

    return ERROR_NONE;
}
