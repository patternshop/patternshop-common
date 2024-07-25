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

const int PsWinProjectCx::bloc_count_size = 22;
const int PsWinProjectCx::item_count_size = 29;

PsWinProjectCx::PsWinProjectCx(PsWin* psWin) : psWin(psWin)
{
	this->bDragging = false;
	this->selected = NULL;
	this->dragLast = NULL;
	this->dragBefore = NULL;
	this->dragTopmost = false;
}

PsWinProjectCx::~PsWinProjectCx()
{
	std::map<uint32, SoftwareBuffer*>::iterator i = this->imageList.begin();
	for (; i != this->imageList.end(); ++i)
	{
		delete i->second;
		i->second = 0;
	}
	this->imageList.clear();
}

void PsWinProjectCx::OnLeftMouseButtonDown(PsPoint point)
{
	// PsRect k;
	// scrollbar->GetClientRect(&k);
	this->fromPoint = point;
	PsProjectController* project_controller = PsController::Instance().project_controller;
	if (!project_controller) return;
	if (project_controller->bPatternsIsSelected)
		project_controller->bPatternsIsSelected = false;
	if (point.y > 0 && point.x < this->psWin->iWidth - this->scrollbar->GetWidth()
		&& point.y < this->totalHSize - this->scrollbar->GetPos())
	{
		if (project_controller)
		{
			this->ypos_precalc = 0 - this->scrollbar->GetPos();
			ImageList::reverse_iterator image = project_controller->images.rbegin();
			for (; image != project_controller->images.rend(); image++)
				this->OnLButtonDownIn(point, *image);
			MatrixList::reverse_iterator matrix = project_controller->matrices.rbegin();
			for (; matrix != project_controller->matrices.rend(); matrix++)
			{
				if (point.y > this->ypos_precalc &&
					point.y < this->ypos_precalc + this->bloc_count_size)
				{
					if (point.x > 42)
					{
						project_controller->SelectMatrix(*matrix);
						this->bDrawDragging = false;
						this->bDragging = true;
						this->dragBefore = NULL;
						PsController::Instance().UpdateWindow();
					}
					else if (point.x > 25)
					{
						if (!this->openCloseMap[*matrix]) this->openCloseMap[*matrix] = OPEN;
						if (this->openCloseMap[*matrix] == OPEN) this->openCloseMap[*matrix] = CLOSE;
						else this->openCloseMap[*matrix] = OPEN;
						this->Update();
					}
					else
					{
						if ((*matrix)->hide) (*matrix)->hide = false;
						else (*matrix)->hide = true;
						this->Update();
						PsController::Instance().UpdateWindow();
					}
					return;
				}
				this->ypos_precalc += this->bloc_count_size;
				if (this->openCloseMap[*matrix] != CLOSE)
				{
					ImageList::reverse_iterator image = (*matrix)->images.rbegin();
					for (; image != (*matrix)->images.rend(); image++)
						this->OnLButtonDownIn(point, *image);
				}
			}

			// pattern
			if (project_controller->pattern)
			{
				if (point.y > this->ypos_precalc &&
					point.y < this->ypos_precalc + this->bloc_count_size)
				{
					PsPattern* pattern = project_controller->pattern;
					if (point.x < 25)
					{
						if (pattern->hide) pattern->hide = false;
						else pattern->hide = true;
					}
					else
					{
						/*
						SoftwareBuffer* img = this->imageList[pattern->GetAutoGenId()];
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
						project_controller->bPatternsIsSelected = true;
						this->selected = NULL;
						if (PsController::Instance().active)
						PsController::Instance().UpdateWindow();
						}*/
					}
					this->Update();
					PsController::Instance().UpdateWindow();
				}
				this->ypos_precalc += this->item_count_size;
			}

			// background color
			if (point.y > this->ypos_precalc && point.y < this->ypos_precalc + this->bloc_count_size)
			{
				if (point.x < 25)
				{
					if (project_controller->bHideColor) project_controller->bHideColor = false;
					else project_controller->bHideColor = true;
				}
				else
				{
					PsDlgColor dlg;
					dlg.SetColor(project_controller->iColor[0], project_controller->iColor[1], project_controller->iColor[2]);
					if (dlg.ShowModal())
					{
						project_controller->iColor[0] = dlg.GetColorRValue();
						project_controller->iColor[1] = dlg.GetColorGValue();
						project_controller->iColor[2] = dlg.GetColorBValue();
					}
				}
				this->Update();
				PsController::Instance().UpdateWindow();
			}
		}
	}
}

void PsWinProjectCx::OnLButtonDownIn(PsPoint point, PsImage* image)
{
	if (point.y > this->ypos_precalc &&
		point.y < this->ypos_precalc + this->item_count_size)
	{
		if (point.x > 25)
		{
			PsProjectController* project_controller = PsController::Instance().project_controller;
			project_controller->SelectImage(image);
			this->bDrawDragging = false;
			this->bDragging = true;
			this->dragBefore = NULL;
			this->Update();
			PsController::Instance().UpdateWindow();
			this->OnMyMouseMove(point);
			return;
		}
		else
		{
			if (image->hide) image->hide = false;
			else image->hide = true;
			this->Update();
			PsController::Instance().UpdateWindow();
		}
	}
	this->ypos_precalc += this->item_count_size;
}
