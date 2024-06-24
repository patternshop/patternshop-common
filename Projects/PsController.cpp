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

PsController*	PsController::instance = 0;

/*
** Constructeur
** Initialement, aucune fenêtre ni aucun projet actif (active = 0, project = 0), l'outil de base
** est l'outil de modification.
*/
PsController::PsController() :
#ifdef _WINDOWS
	active(0),
#endif /* _WINDOWS */
	tool(TOOL_MODIFY),
	project(0)
{
#ifndef _MACOSX
	cursor[CURSOR_DEFAULT] = AfxGetApp()->LoadCursor(IDC_DEFAULT);
	cursor[CURSOR_MAGNIFY1] = AfxGetApp()->LoadCursor(IDC_MAGNIFY1);
	cursor[CURSOR_MAGNIFY2] = AfxGetApp()->LoadCursor(IDC_MAGNIFY2);
	cursor[CURSOR_MAGNIFY3] = AfxGetApp()->LoadCursor(IDC_MAGNIFY3);
	cursor[CURSOR_MOVE] = AfxGetApp()->LoadCursor(IDC_MOVE);
	cursor[CURSOR_ROTATE1] = AfxGetApp()->LoadCursor(IDC_ROTATE1);
	cursor[CURSOR_ROTATE2] = AfxGetApp()->LoadCursor(IDC_ROTATE2);
	cursor[CURSOR_ROTATE3] = AfxGetApp()->LoadCursor(IDC_ROTATE3);
	cursor[CURSOR_ROTATE4] = AfxGetApp()->LoadCursor(IDC_ROTATE4);
	cursor[CURSOR_SCROLL1] = AfxGetApp()->LoadCursor(IDC_HAND1);
	cursor[CURSOR_SCROLL2] = AfxGetApp()->LoadCursor(IDC_HAND2);
	cursor[CURSOR_SIZE1] = AfxGetApp()->LoadCursor(IDC_SIZE2);
	cursor[CURSOR_SIZE2] = AfxGetApp()->LoadCursor(IDC_SIZE1);
	cursor[CURSOR_TORSIO1] = AfxGetApp()->LoadCursor(IDC_HAND1);
	cursor[CURSOR_TORSIO2] = AfxGetApp()->LoadCursor(IDC_HAND2);
#endif
 
	option[OPTION_AUTOMATIC] = true;
//	option[OPTION_BOX_MOVE] = true;
	option[OPTION_BOX_SHOW] = true;
	option[OPTION_CONSTRAIN] = false;
	option[OPTION_DOCUMENT_BLEND] = true;
	option[OPTION_DOCUMENT_SHOW] = true;
	option[OPTION_HIGHLIGHT_SHOW] = true;
	option[OPTION_REFLECT] = false;

  bMouseButtonIsDown = false;
}

PsController::~PsController()
{
}

PsController&	PsController::Instance()
{
	if(!instance)
		instance = new PsController();

	return *instance;
}

void	PsController::Delete()
{
	if(instance)
	{
		delete instance;
		instance = 0;
	}
}

void PsController::UpdateWindow()
{
  if(active)
    active->Update();
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
** Récupere l'état d'une option; dans certains cas, l'outil actif prévaut sur l'option(par exemple
** on ne voit pas le cadre bleu ni les boites de selection quand on zoome ou qu'on scrolle le projet)
** Le booleen "real_state", à false par défaut, récupere l'état de l'option sans prendre en compte
** l'outil.
*/
bool	PsController::GetOption(Option index, bool real_state) const
{
	if(!real_state)
		switch(index)
		{
			case OPTION_BOX_SHOW:
			case OPTION_HIGHLIGHT_SHOW:
				if(tool != TOOL_MAGNIFY_ZOOM && tool != TOOL_SCROLL_DRAG)
					return option[index];
				//else
					//return !option[OPTION_BOX_MOVE];
		}

	return option[index];
}

/*
** L'utilisateur a cliqué avec le bouton gauche(num = 0) ou droit(num = 1)
** à la position x, y sur la fenêtre du projet.
*/
void	PsController::MouseClick(int num, int x, int y)
{
	if(!project)
		return;

  bMouseButtonIsDown = true;

	project->LogInit();
	prev_x = x;
	prev_y = y;

	switch(tool)
	{
		case TOOL_MAGNIFY:
			if(num == 0)
				tool = project->ToolMagnifyStart();
			break;

		case TOOL_MODIFY:
			if(num == 0)
				tool = project->ToolModifyScan(x, y, true);
			break;

		case TOOL_SCROLL:
			if(num == 0)
			{
				SetCursor(CURSOR_SCROLL2);
				tool = project->ToolScrollStart();
			}
			break;
	}
}

/*
** L'utilisateur déplace la souris à la position x, y sur la fenêtre du projet.
*/
void	PsController::MouseMove(int x, int y)
{
	if(!project)
		return;

	switch(tool)
	{
		case TOOL_MAGNIFY_ZOOM:
			project->ToolMagnifyDrag(y, prev_x, prev_y);
			break;

		case TOOL_MODIFY:
			project->ToolModifyScan(x, y, false);
			break;

		case TOOL_MODIFY_MOVE:
		case TOOL_MODIFY_ROTATE:
		case TOOL_MODIFY_SIZE:
		case TOOL_MODIFY_TORSIO:
			project->ToolModifyMove(x, y, prev_x, prev_y, tool);
			break;

		case TOOL_SCROLL_DRAG:
			project->ToolScrollDrag(x, y, prev_x, prev_y);
			break;
	}
}

/*
** L'utilisateur relâche un bouton de la souris(voir paramètres de MouseClick).
*/
void	PsController::MouseRelease(int num, int x, int y)
{
	if(!project)
		return;

  bMouseButtonIsDown = false;

	switch(tool)
	{
		case TOOL_MAGNIFY_ZOOM:
			if(num == 0)
			{
				SetCursor(CURSOR_MAGNIFY1);
				tool = TOOL_MAGNIFY;
			}
			break;

		case TOOL_MODIFY_MOVE:
		case TOOL_MODIFY_ROTATE:
		case TOOL_MODIFY_SIZE:
		case TOOL_MODIFY_TORSIO:
			if(num == 0)
				tool = TOOL_MODIFY;
			break;

		case TOOL_SCROLL_DRAG:
			if(num == 0)
			{
				SetCursor(CURSOR_SCROLL1);
				tool = TOOL_SCROLL;
			}
			break;
	}
}

#ifdef _WINDOWS
void	PsController::SetActive(CPatternshopView* view)
{
	if(view != this->active || view && view->project != project)
	{
		if(view)
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
		UpdateDialogProject();
		UpdateDialogOverview(false);
	}
}
#else /* _MACOSX */
void	PsController::SetActive(PsProject* p)
{
	this->project = p;
	PsWinProject::Instance().Update();
	if (dlgPropreties)
	{
		dlgPropreties->UpdateInformation(NULL);
		//dlgPropreties->FocusMatrixInformation();
	}
}
#endif

/*
** Change le curseur courant.
*/
void	PsController::SetCursor(PsCursor num)
{
  #ifdef _WINDOWS
	if(active)
		active->SetMouseCursor(cursor[num]);
  #else /* _MACOSX */
	SetMacCursor(num);
#endif
}

/*
** Active ou désactive une option.
*/
void	PsController::SetOption(Option index, bool value)
{
	option[index] = value;
}

/*
** Change l'indicateur de progression
*/
void	PsController::SetProgress(int pos)
{
  #ifdef _WINDOWS
	CMainFrame*	main =(CMainFrame*)theApp.GetMainWnd();
	if(main)
		main->ProgressBar(pos);
  #endif /* _WINDOWS */
}

/*
** Change l'outil courant.
*/
void	PsController::SetTool(Tool tool)
{

	switch(tool)
	{
		case TOOL_MAGNIFY:
			SetCursor(CURSOR_MAGNIFY1);
			break;

		case TOOL_MODIFY:
			SetCursor(CURSOR_DEFAULT);
			break;

		case TOOL_SCROLL:
			SetCursor(CURSOR_SCROLL1);
			break;
	}

  #ifdef _WINDOWS
	if(active) 
		active->Update();
  #endif /* _WINDOWS */

	prev_tool = this->tool;
	this->tool = tool;
	
	if (this->tool == TOOL_MODIFY)
		prev_tool = TOOL_MODIFY;
}
