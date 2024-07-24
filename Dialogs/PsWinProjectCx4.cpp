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

void PsWinProjectCx::GenericUpdate()
{
	PsProjectController* project = PsController::Instance().project;

	if (!project) this->scrollbar->Disable();

	this->psWin->SetPenColor(212, 208, 200);
	this->psWin->SetBrushColor(212, 208, 200);
	this->psWin->DrawRectangle(0, 0, this->psWin->iWidth, this->psWin->iHeight);
	if (project)
	{
		if (project->matrix && project->matrix != this->selected && !project->image)
			this->selected = project->matrix;
		if (project->image && project->image != this->selected)
			this->selected = project->image;
		this->ypos_precalc = 0 - this->scrollbar->GetPos();
		this->bloc_count = 0;
		this->item_count = 0;
		if (this->bDragging)
		{
			this->dragBefore = NULL;
			this->dragLast = NULL;
			this->dragTopmost = false;
		}
		this->imageNameCount = 1; // FIXME
		ImageList::reverse_iterator image = project->images.rbegin();
		for (; image != project->images.rend(); image++)
		{
			this->DrawImageBloc(*image);
			this->imageNameCount++;
			this->item_count++;
		}
		this->matNameCount = 1; // FIXME
		MatrixList::reverse_iterator matrix = project->matrices.rbegin();
		for (; matrix != project->matrices.rend(); matrix++)
		{
			this->DrawMatrixBloc(*matrix);
			if (this->openCloseMap[*matrix] != CLOSE)
			{
				this->motifNameCount = 1; // FIXME
				ImageList::reverse_iterator image = (*matrix)->images.rbegin();
				for (; image != (*matrix)->images.rend();)
				{
					this->DrawImageBloc(*image);
					image++;
					if (image != (*matrix)->images.rend())
					{
						this->psWin->SetDashBlackPen();
						this->psWin->MovePenTo(26 + 15, this->ypos_precalc - 1);
						this->psWin->DrawLineTo(this->psWin->iWidth, this->ypos_precalc - 1);
					}
					this->item_count++;
					this->motifNameCount++;
				}
			}
			if (this->bDragging && !this->dragBefore
				&& this->draggingPoint.y < this->ypos_precalc + this->bloc_count_size
				&& this->draggingPoint.y > this->ypos_precalc)
			{
				this->DrawInsertionCaret();
				this->dragLast = *matrix;
			}
		}
		int swap = this->ypos_precalc;
		this->ypos_precalc = -this->scrollbar->GetPos();
		if (this->bDragging && dynamic_cast<PsImage*>(this->selected)
			&& this->draggingPoint.y < this->ypos_precalc + this->item_count_size
			&& this->draggingPoint.y > this->ypos_precalc)
		{
			this->DrawInsertionCaret();
			this->dragTopmost = true;
		}
		this->ypos_precalc = swap;

		this->DrawBackgroundBloc();
		this->psWin->SetPenColor(0, 0, 0);
		this->psWin->MovePenTo(0, 0);
		this->psWin->DrawLineTo(this->psWin->iWidth, 0);
		this->psWin->MovePenTo(0, this->ypos_precalc);
		this->psWin->DrawLineTo(this->psWin->iWidth, this->ypos_precalc);
		this->psWin->MovePenTo(25, 0);
		this->psWin->DrawLineTo(25, this->ypos_precalc);

		this->totalHSize = (this->item_count + 1) * this->item_count_size + this->bloc_count * this->bloc_count_size;
		if (project->pattern) this->totalHSize += this->item_count_size;

		if (this->totalHSize - (int)this->psWin->iHeight > 0)
		{
			this->scrollbar->Enable();
			this->scrollbar->SetSize(this->totalHSize - this->psWin->iHeight);
		}
		else this->scrollbar->Disable();

	}
	if (this->bDragging && this->bDrawDragging)
	{
		PsRect m;
		m.left = 0;
		m.right = this->psWin->iWidth - this->s.right - 1;
		m.top = this->draggingPoint.y - this->item_count_size / 2;
		m.bottom = this->draggingPoint.y + this->item_count_size / 2;
		if (m.top < 0) m.top = 0;
		if (m.bottom < 0) m.bottom = 0;
		this->psWin->SetDashBlackPen();
		this->psWin->MovePenTo(m.left, m.top);
		this->psWin->DrawLineTo(m.right, m.top);
		this->psWin->DrawLineTo(m.right, m.bottom);
		this->psWin->DrawLineTo(m.left, m.bottom);
		this->psWin->DrawLineTo(m.left, m.top);
	}

}
