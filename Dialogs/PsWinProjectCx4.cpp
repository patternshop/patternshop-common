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
	PsProject* project = PsController::Instance().project;

	if (!project) scrollbar->Disable();

	psWin->SetPenColor(212, 208, 200);;
	psWin->SetBrushColor(212, 208, 200);
	psWin->DrawRectangle(0, 0, psWin->iWidth, psWin->iHeight);
	if (project)
	{
		if (project->matrix && project->matrix != selected && !project->image)
			selected = project->matrix;
		if (project->image && project->image != selected)
			selected = project->image;
		ypos_precalc = 0 - scrollbar->GetPos();
		bloc_count = 0;
		item_count = 0;
		if (bDragging)
		{
			dragBefore = NULL;
			dragLast = NULL;
			dragTopmost = false;
		}
		imageNameCount = 1; // FIXME
		ImageList::reverse_iterator image = project->images.rbegin();
		for (; image != project->images.rend(); image++)
		{
			DrawImageBloc(*image);
			imageNameCount++;
			item_count++;
		}
		matNameCount = 1; // FIXME
		MatrixList::reverse_iterator matrix = project->matrices.rbegin();
		for (; matrix != project->matrices.rend(); matrix++)
		{
			DrawMatrixBloc(*matrix);
			if (openCloseMap[*matrix] != CLOSE)
			{
				motifNameCount = 1; // FIXME
				ImageList::reverse_iterator image = (*matrix)->images.rbegin();
				for (; image != (*matrix)->images.rend();)
				{
					DrawImageBloc(*image);
					image++;
					if (image != (*matrix)->images.rend())
					{
						psWin->SetDashBlackPen();
						psWin->MovePenTo(26 + 15, ypos_precalc - 1);
						psWin->DrawLineTo(psWin->iWidth, ypos_precalc - 1);
					}
					item_count++;
					motifNameCount++;
				}
			}
			if (bDragging && !dragBefore// && selected != *matrix
				&& draggingPoint.y < ypos_precalc + bloc_count_size
				&& draggingPoint.y > ypos_precalc)
			{
				DrawInsertionCaret();
				dragLast = *matrix;
			}
		}
		int swap = ypos_precalc;
		ypos_precalc = -scrollbar->GetPos();
		if (bDragging && dynamic_cast<PsImage*>(selected)
			&& draggingPoint.y < ypos_precalc + item_count_size
			&& draggingPoint.y > ypos_precalc)
		{
			DrawInsertionCaret();
			dragTopmost = true;
		}
		ypos_precalc = swap;

		DrawBackgroundBloc();
		psWin->SetPenColor(0, 0, 0);
		psWin->MovePenTo(0, 0);
		psWin->DrawLineTo(psWin->iWidth, 0);
		psWin->MovePenTo(0, ypos_precalc);
		psWin->DrawLineTo(psWin->iWidth, ypos_precalc);
		psWin->MovePenTo(25, 0);
		psWin->DrawLineTo(25, ypos_precalc);

		totalHSize = (item_count + 1) * item_count_size + bloc_count * bloc_count_size;
		if (project->pattern) totalHSize += item_count_size;

		if (totalHSize - (int)psWin->iHeight > 0)
		{
			scrollbar->Enable();
			scrollbar->SetSize(totalHSize - psWin->iHeight);
		}
		else scrollbar->Disable();

	}
	if (bDragging && bDrawDragging)
	{
		PsRect m;
		m.left = 0;
		m.right = psWin->iWidth - s.right - 1;
		m.top = draggingPoint.y - item_count_size / 2;
		m.bottom = draggingPoint.y + item_count_size / 2;
		if (m.top < 0) m.top = 0;
		if (m.bottom < 0) m.bottom = 0;
		psWin->SetDashBlackPen();
		psWin->MovePenTo(m.left, m.top);
		psWin->DrawLineTo(m.right, m.top);
		psWin->DrawLineTo(m.right, m.bottom);
		psWin->DrawLineTo(m.left, m.bottom);
		psWin->DrawLineTo(m.left, m.top);
	}

}

