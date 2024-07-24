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

void PsWinProjectCx::DrawBackgroundBloc()
{
	PsProject* project = PsController::Instance().project;

	if (project)
	{
		if (project->pattern)
		{
			PsPattern* pattern = project->pattern;
			// pattern
			if (ypos_precalc + item_count_size > 0)
			{
				if (!pattern->hide) viewImage.BitBlt((*psWin), 4, ypos_precalc + 4);
				else boxImage.BitBlt((*psWin), 4, ypos_precalc + 4);
				psWin->SetBrushColor(212, 208, 200);
				psWin->SetPenColor(255, 255, 255);
				psWin->DrawRectangle(26, ypos_precalc, psWin->iWidth, ypos_precalc + item_count_size);
				SoftwareBuffer* img = imageList[pattern->GetAutoGenId()];
				if (!img)
					img = loadThumb(&(pattern->texture));
				int p = (32 - img->GetWidth()) / 2 + 26;
				int u = (item_count_size - img->GetHeight()) / 2 + ypos_precalc;
				/*if (project->bPatternsIsSelected)
				{
				SetBrushColor(10, 36, 106);
				SetPenColor(10, 36, 106);
				DrawRectangle(p - 2, u - 2, p + img->GetWidth() + 2, u + img->GetHeight() + 2);
				}*/
				img->BitBlt((*psWin), p, u);
				p += img->GetWidth() + 5;
				//spot.BitBlt(*this, p, ypos_precalc + 5);
				//p += 18 + 5;
				PsRect t;
				t.left = p; t.top = ypos_precalc;
				t.right = psWin->iWidth; t.bottom = ypos_precalc + item_count_size;
				psWin->SetTextColor(0, 0, 0);
				psWin->DrawText(GetLabel(LABEL_PATTERN), t, PsWin::PSFONT_NORMAL);
			}
			ypos_precalc += item_count_size;
		}

		// uniform color
		if (ypos_precalc + item_count_size > 0)
		{
			if (!project->bHideColor) viewImage.BitBlt((*psWin), 4, ypos_precalc + 4);
			else boxImage.BitBlt((*psWin), 4, ypos_precalc + 4);
			psWin->SetBrushColor(212, 208, 200);
			psWin->SetPenColor(255, 255, 255);
			psWin->DrawRectangle(26, ypos_precalc, psWin->iWidth, ypos_precalc + item_count_size);
			int p = 15 + 26;
			int u = ypos_precalc;

			psWin->SetBrushColor(project->iColor[0], project->iColor[1], project->iColor[2]);
			psWin->SetPenColor(0, 0, 0);
			psWin->DrawRectangle(35, ypos_precalc + 5, 70, ypos_precalc + item_count_size - 5);

			PsRect t;
			t.left = 80; t.top = ypos_precalc;
			t.right = psWin->iWidth; t.bottom = ypos_precalc + item_count_size;
			psWin->SetTextColor(0, 0, 0);
			psWin->DrawText(GetLabel(LABEL_BACKGROUND), t, PsWin::PSFONT_NORMAL);
		}
		ypos_precalc += item_count_size;
	}
}

void PsWinProjectCx::DrawInsertionCaret()
{
	psWin->SetPenColor(0, 0, 0);
	psWin->SetBrushColor(0, 0, 0);
	psWin->MovePenTo(0, ypos_precalc);
	psWin->DrawLineTo(psWin->iWidth, ypos_precalc);
	psWin->MovePenTo(0, ypos_precalc - 2);
	psWin->DrawLineTo(psWin->iWidth, ypos_precalc - 2);
	PsPoint ptsr[3], * pts = ptsr;
	pts[0].x = 0; pts[0].y = ypos_precalc - 2;
	pts[1].x = 0; pts[1].y = ypos_precalc - 5;
	pts[2].x = 3; pts[2].y = ypos_precalc - 2;
	psWin->DrawPolygon(pts, 3);
	pts[0].x = 0; pts[0].y = ypos_precalc;
	pts[1].x = 0; pts[1].y = ypos_precalc + 3;
	pts[2].x = 3; pts[2].y = ypos_precalc;
	psWin->DrawPolygon(pts, 3);
	pts[0].x = psWin->iWidth - s.right - 1; pts[0].y = ypos_precalc - 2;
	pts[1].x = psWin->iWidth - s.right - 1; pts[1].y = ypos_precalc - 5;
	pts[2].x = psWin->iWidth - s.right - 4; pts[2].y = ypos_precalc - 2;
	psWin->DrawPolygon(pts, 3);
	pts[0].x = psWin->iWidth - s.right - 1; pts[0].y = ypos_precalc;
	pts[1].x = psWin->iWidth - s.right - 1; pts[1].y = ypos_precalc + 3;
	pts[2].x = psWin->iWidth - s.right - 4; pts[2].y = ypos_precalc;
	psWin->DrawPolygon(pts, 3);
}
