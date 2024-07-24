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
#include "PsController.h"

#ifdef _WINDOWS
# include "PatternshopView.h"
# include "Patternshop.h"
# include "MainFrm.h"

#else /* _MACOSX */
# include "PsMacView.h"
# include "PsMacCursor.h"
#endif

#include "PsProject.h"
#include "PsWinPropertiesCx.h"
#include "PsWinProject.h"
#include "PsWinOverview.h"

PsController* PsController::instance = 0;

/*
** Constructor
** Initially, no window or active project (active = 0, project = 0), the default tool
** is the modification tool.
*/
PsController::PsController() :
#ifdef _WINDOWS
	active(0),
#endif /* _WINDOWS */
	tool(TOOL_MODIFY),
	project(0)
{
#ifndef _MACOSX
	this->cursor[CURSOR_DEFAULT] = AfxGetApp()->LoadCursor(IDC_DEFAULT);
	this->cursor[CURSOR_MAGNIFY1] = AfxGetApp()->LoadCursor(IDC_MAGNIFY1);
	this->cursor[CURSOR_MAGNIFY2] = AfxGetApp()->LoadCursor(IDC_MAGNIFY2);
	this->cursor[CURSOR_MAGNIFY3] = AfxGetApp()->LoadCursor(IDC_MAGNIFY3);
	this->cursor[CURSOR_MOVE] = AfxGetApp()->LoadCursor(IDC_MOVE);
	this->cursor[CURSOR_ROTATE1] = AfxGetApp()->LoadCursor(IDC_ROTATE1);
	this->cursor[CURSOR_ROTATE2] = AfxGetApp()->LoadCursor(IDC_ROTATE2);
	this->cursor[CURSOR_ROTATE3] = AfxGetApp()->LoadCursor(IDC_ROTATE3);
	this->cursor[CURSOR_ROTATE4] = AfxGetApp()->LoadCursor(IDC_ROTATE4);
	this->cursor[CURSOR_SCROLL1] = AfxGetApp()->LoadCursor(IDC_HAND1);
	this->cursor[CURSOR_SCROLL2] = AfxGetApp()->LoadCursor(IDC_HAND2);
	this->cursor[CURSOR_SIZE1] = AfxGetApp()->LoadCursor(IDC_SIZE2);
	this->cursor[CURSOR_SIZE2] = AfxGetApp()->LoadCursor(IDC_SIZE1);
	this->cursor[CURSOR_TORSIO1] = AfxGetApp()->LoadCursor(IDC_HAND1);
	this->cursor[CURSOR_TORSIO2] = AfxGetApp()->LoadCursor(IDC_HAND2);
#endif

	this->option[OPTION_AUTOMATIC] = true;
	//	this->option[OPTION_BOX_MOVE] = true;
	this->option[OPTION_BOX_SHOW] = true;
	this->option[OPTION_CONSTRAIN] = false;
	this->option[OPTION_DOCUMENT_BLEND] = true;
	this->option[OPTION_DOCUMENT_SHOW] = true;
	this->option[OPTION_HIGHLIGHT_SHOW] = true;
	this->option[OPTION_REFLECT] = false;

	this->bMouseButtonIsDown = false;
}

PsController::~PsController()
{
}

PsController& PsController::Instance()
{
	if (!instance)
		instance = new PsController();

	return *instance;
}

void PsController::Delete()
{
	if (instance)
	{
		delete instance;
		instance = 0;
	}
}

void PsController::UpdateWindow()
{
	if (this->active)
		this->active->Update();
}

void PsController::UpdateDialogProject()
{
	PsWinProject::Instance().Update();
}

void PsController::UpdateDialogOverview(bool bQuick)
{
	PsWinProject::Instance().Update();
#ifdef _WINDOWS
	if (bQuick) PsWinOverview::Instance().Invalidate(true);
	else PsWinOverview::Instance().Update();
#else /* _MACOSX */
	PsWinOverview::Instance().Update();
#endif
}


/*
** Retrieves the state of an option; in some cases, the active tool prevails over the option (for example,
** you do not see the blue frame or the selection boxes when zooming or scrolling the project)
** The boolean "real_state", false by default, retrieves the state of the option without taking into account
** the tool.
*/
bool PsController::GetOption(Option index, bool real_state) const
{
	if (!real_state)
		switch (index)
		{
		case OPTION_BOX_SHOW:
		case OPTION_HIGHLIGHT_SHOW:
			if (this->tool != TOOL_MAGNIFY_ZOOM && this->tool != TOOL_SCROLL_DRAG)
				return this->option[index];
			//else
				//return !this->option[OPTION_BOX_MOVE];
		}

	return this->option[index];
}

/*
** The user clicked with the left button (num = 0) or right button (num = 1)
** at position x, y on the project window.
*/
void PsController::MouseClick(int num, int x, int y)
{
	if (!this->project)
		return;

	this->bMouseButtonIsDown = true;

	this->project->LogInit();
	this->prev_x = x;
	this->prev_y = y;

	switch (this->tool)
	{
	case TOOL_MAGNIFY:
		if (num == 0)
			this->tool = this->project->ToolMagnifyStart();
		break;

	case TOOL_MODIFY:
		if (num == 0)
			this->tool = this->project->ToolModifyScan(x, y, true);
		break;

	case TOOL_SCROLL:
		if (num == 0)
		{
			this->SetCursor(CURSOR_SCROLL2);
			this->tool = this->project->ToolScrollStart();
		}
		break;
	}
}

/*
** The user moves the mouse to position x, y on the project window.
*/
void PsController::MouseMove(int x, int y)
{
	if (!this->project)
		return;

	switch (this->tool)
	{
	case TOOL_MAGNIFY_ZOOM:
		this->project->ToolMagnifyDrag(y, this->prev_x, this->prev_y);
		break;

	case TOOL_MODIFY:
		this->project->ToolModifyScan(x, y, false);
		break;

	case TOOL_MODIFY_MOVE:
	case TOOL_MODIFY_ROTATE:
	case TOOL_MODIFY_SIZE:
	case TOOL_MODIFY_TORSIO:
		this->project->ToolModifyMove(x, y, this->prev_x, this->prev_y, this->tool);
		break;

	case TOOL_SCROLL_DRAG:
		this->project->ToolScrollDrag(x, y, this->prev_x, this->prev_y);
		break;
	}
}

/*
** The user releases a mouse button (see MouseClick parameters).
*/
void PsController::MouseRelease(int num, int x, int y)
{
	if (!this->project)
		return;

	this->bMouseButtonIsDown = false;

	switch (this->tool)
	{
	case TOOL_MAGNIFY_ZOOM:
		if (num == 0)
		{
			this->SetCursor(CURSOR_MAGNIFY1);
			this->tool = TOOL_MAGNIFY;
		}
		break;

	case TOOL_MODIFY_MOVE:
	case TOOL_MODIFY_ROTATE:
	case TOOL_MODIFY_SIZE:
	case TOOL_MODIFY_TORSIO:
		if (num == 0)
			this->tool = TOOL_MODIFY;
		break;

	case TOOL_SCROLL_DRAG:
		if (num == 0)
		{
			this->SetCursor(CURSOR_SCROLL1);
			this->tool = TOOL_SCROLL;
		}
		break;
	}
}

#ifdef _WINDOWS
void PsController::SetActive(CPatternshopView* view)
{
	if (view != this->active || view && view->project != this->project)
	{
		if (view)
		{
			this->active = view;
			this->project = view->project;
		}
		else
		{
			this->active = NULL;
			this->project = NULL;
		}
		dlgPropreties->FocusMatrixInformation();
		this->UpdateDialogProject();
		this->UpdateDialogOverview(false);
	}
}
#else /* _MACOSX */
void PsController::SetActive(PsProject* p)
{
	this->project = p;
	PsWinProject::Instance().Update();
	if (this->dlgPropreties)
	{
		this->dlgPropreties->UpdateInformation(NULL);
		//this->dlgPropreties->FocusMatrixInformation();
	}
}
#endif

/*
** Change the current cursor.
*/
void PsController::SetCursor(PsCursor num)
{
#ifdef _WINDOWS
	if (this->active)
		this->active->SetMouseCursor(this->cursor[num]);
#else /* _MACOSX */
	SetMacCursor(num);
#endif
}

/*
** Enable or disable an option.
*/
void PsController::SetOption(Option index, bool value)
{
	this->option[index] = value;
}

/*
** Change the progress indicator
*/
void PsController::SetProgress(int pos)
{
#ifdef _WINDOWS
	CMainFrame* main = (CMainFrame*)theApp.GetMainWnd();
	if (main)
		main->ProgressBar(pos);
#endif /* _WINDOWS */
}

/*
** Change the current tool.
*/
void PsController::SetTool(Tool tool)
{

	switch (tool)
	{
	case TOOL_MAGNIFY:
		this->SetCursor(CURSOR_MAGNIFY1);
		break;

	case TOOL_MODIFY:
		this->SetCursor(CURSOR_DEFAULT);
		break;

	case TOOL_SCROLL:
		this->SetCursor(CURSOR_SCROLL1);
		break;
	}

#ifdef _WINDOWS
	if (this->active)
		this->active->Update();
#endif /* _WINDOWS */

	this->prev_tool = this->tool;
	this->tool = tool;

	if (this->tool == TOOL_MODIFY)
		this->prev_tool = TOOL_MODIFY;
}
