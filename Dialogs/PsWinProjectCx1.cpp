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
#include "PsAction.h"
#include "PsController.h"
#include "PsWinProject.h"
#include "PsDlgInfoLight.h"
#include "PsDlgColor.h"
#include "FreeImage.h"

#ifdef _MACOSX
#include "PsMacTools.h"
#endif

#include <assert.h>

const int	PsWinProjectCx::bloc_count_size = 22;
const int	PsWinProjectCx::item_count_size = 29;

PsWinProjectCx::PsWinProjectCx(PsWin* psWin) : psWin(psWin)
{
	bDragging = false;
	selected = NULL;
	dragLast = NULL;
	dragBefore = NULL;
	dragTopmost = false;
}

PsWinProjectCx::~PsWinProjectCx()
{
	std::map<uint32, SoftwareBuffer*>::iterator i = imageList.begin();
	for (; i != imageList.end(); ++i)
	{
		delete i->second;
		i->second = 0;
	}
	imageList.clear();

}

void PsWinProjectCx::OnLeftMouseButtonDown(PsPoint point)
{
	//PsRect k;
	//scrollbar->GetClientRect(&k);
	fromPoint = point;
	PsProject* project = PsController::Instance().project;
	if (!project) return;
	if (project->bPatternsIsSelected)
		project->bPatternsIsSelected = false;
	if (point.y > 0 && point.x < psWin->iWidth - scrollbar->GetWidth()
		&& point.y < totalHSize - scrollbar->GetPos())
	{
		if (project)
		{
			ypos_precalc = 0 - scrollbar->GetPos();
			ImageList::reverse_iterator image = project->images.rbegin();
			for (; image != project->images.rend(); image++)
				OnLButtonDownIn(point, *image);
			MatrixList::reverse_iterator matrix = project->matrices.rbegin();
			for (; matrix != project->matrices.rend(); matrix++)
			{
				if (point.y > ypos_precalc &&
					point.y < ypos_precalc + bloc_count_size)
				{
					if (point.x > 42)
					{
						project->SelectMatrix(*matrix);
						bDrawDragging = false;
						bDragging = true;
						dragBefore = NULL;
						PsController::Instance().UpdateWindow();
					}
					else if (point.x > 25)
					{
						if (!openCloseMap[*matrix]) openCloseMap[*matrix] = OPEN;
						if (openCloseMap[*matrix] == OPEN) openCloseMap[*matrix] = CLOSE;
						else openCloseMap[*matrix] = OPEN;
						Update();
					}
					else
					{
						if ((*matrix)->hide) (*matrix)->hide = false;
						else (*matrix)->hide = true;
						Update();
						PsController::Instance().UpdateWindow();
					}
					return;
				}
				ypos_precalc += bloc_count_size;
				if (openCloseMap[*matrix] != CLOSE)
				{
					ImageList::reverse_iterator image = (*matrix)->images.rbegin();
					for (; image != (*matrix)->images.rend(); image++)
						OnLButtonDownIn(point, *image);
				}
			}

			// patron
			if (project->pattern)
			{
				if (point.y > ypos_precalc &&
					point.y < ypos_precalc + bloc_count_size)
				{
					PsPattern* pattern = project->pattern;
					if (point.x < 25)
					{
						if (pattern->hide) pattern->hide = false;
						else pattern->hide = true;
					}
					else
					{
						/*
						SoftwareBuffer* img = imageList[pattern->GetAutoGenId()];
						if (!img)
						img = loadThumb(&pattern->texture);
						int p = (32 - img->GetWidth()) / 2 + 26 + img->GetWidth() + 5;
						if (point.x > p && point.x < p + 18)
						{
						PsDlgInfoLight dlg2;
						if (dlg2.ShowModal())
						{
						pattern->SetLinearLight(dlg2.p, dlg2.e);
						PsController::Instance().UpdateWindow();
						}
						}
						*/
						/*else
						{
						project->bPatternsIsSelected = true;
						selected = NULL;
						if (PsController::Instance().active)
						PsController::Instance().UpdateWindow();
						}*/
					}
					Update();
					PsController::Instance().UpdateWindow();
				}
				ypos_precalc += item_count_size;
			}

			// couleur de fond
			if (point.y > ypos_precalc && point.y < ypos_precalc + bloc_count_size)
			{
				if (point.x < 25)
				{
					if (project->bHideColor) project->bHideColor = false;
					else project->bHideColor = true;
				}
				else
				{
					PsDlgColor dlg;
					dlg.SetColor(project->iColor[0], project->iColor[1], project->iColor[2]);
					if (dlg.ShowModal())
					{
						project->iColor[0] = dlg.GetColorRValue();
						project->iColor[1] = dlg.GetColorGValue();
						project->iColor[2] = dlg.GetColorBValue();
					}
				}
				Update();
				PsController::Instance().UpdateWindow();
			}

		}
	}
}

void PsWinProjectCx::OnLButtonDownIn(PsPoint point, PsImage* image)
{
	if (point.y > ypos_precalc &&
		point.y < ypos_precalc + item_count_size)
	{
		if (point.x > 25)
		{
			PsProject* project = PsController::Instance().project;
			project->SelectImage(image);
			bDrawDragging = false;
			bDragging = true;
			dragBefore = NULL;
			Update();
			PsController::Instance().UpdateWindow();
			OnMyMouseMove(point);
			return;
		}
		else
		{
			if (image->hide) image->hide = false;
			else image->hide = true;
			Update();
			PsController::Instance().UpdateWindow();
		}
	}
	ypos_precalc += item_count_size;
}
