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
	PsProjectController* project_controller = PsController::Instance().project_controller;

	if (project_controller)
	{
		if (project_controller->pattern)
		{
			PsPattern* pattern = project_controller->pattern;
			// pattern
			if (this->ypos_precalc + this->item_count_size > 0)
			{
				if (!pattern->hide) this->viewImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 4);
				else this->boxImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 4);
				this->psWin->SetBrushColor(212, 208, 200);
				this->psWin->SetPenColor(255, 255, 255);
				this->psWin->DrawRectangle(26, this->ypos_precalc, this->psWin->iWidth, this->ypos_precalc + this->item_count_size);
				SoftwareBuffer* img = this->imageList[pattern->GetAutoGenId()];
				if (!img)
					img = this->loadThumb(&(pattern->texture));
				int p = (32 - img->GetWidth()) / 2 + 26;
				int u = (this->item_count_size - img->GetHeight()) / 2 + this->ypos_precalc;
				/*if (project_controller->bPatternsIsSelected)
				{
				SetBrushColor(10, 36, 106);
				SetPenColor(10, 36, 106);
				DrawRectangle(p - 2, u - 2, p + img->GetWidth() + 2, u + img->GetHeight() + 2);
				}*/
				img->BitBlt((*this->psWin), p, u);
				p += img->GetWidth() + 5;
				//spot.BitBlt(*this, p, ypos_precalc + 5);
				//p += 18 + 5;
				PsRect t;
				t.left = p; t.top = this->ypos_precalc;
				t.right = this->psWin->iWidth; t.bottom = this->ypos_precalc + this->item_count_size;
				this->psWin->SetTextColor(0, 0, 0);
				this->psWin->DrawText(GetLabel(LABEL_PATTERN), t, PsWin::PSFONT_NORMAL);
			}
			this->ypos_precalc += this->item_count_size;
		}

		// uniform color
		if (this->ypos_precalc + this->item_count_size > 0)
		{
			if (!project_controller->bHideColor) this->viewImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 4);
			else this->boxImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 4);
			this->psWin->SetBrushColor(212, 208, 200);
			this->psWin->SetPenColor(255, 255, 255);
			this->psWin->DrawRectangle(26, this->ypos_precalc, this->psWin->iWidth, this->ypos_precalc + this->item_count_size);
			int p = 15 + 26;
			int u = this->ypos_precalc;

			this->psWin->SetBrushColor(project_controller->iColor[0], project_controller->iColor[1], project_controller->iColor[2]);
			this->psWin->SetPenColor(0, 0, 0);
			this->psWin->DrawRectangle(35, this->ypos_precalc + 5, 70, this->ypos_precalc + this->item_count_size - 5);

			PsRect t;
			t.left = 80; t.top = this->ypos_precalc;
			t.right = this->psWin->iWidth; t.bottom = this->ypos_precalc + this->item_count_size;
			this->psWin->SetTextColor(0, 0, 0);
			this->psWin->DrawText(GetLabel(LABEL_BACKGROUND), t, PsWin::PSFONT_NORMAL);
		}
		this->ypos_precalc += this->item_count_size;
	}
}

void PsWinProjectCx::DrawInsertionCaret()
{
	this->psWin->SetPenColor(0, 0, 0);
	this->psWin->SetBrushColor(0, 0, 0);
	this->psWin->MovePenTo(0, this->ypos_precalc);
	this->psWin->DrawLineTo(this->psWin->iWidth, this->ypos_precalc);
	this->psWin->MovePenTo(0, this->ypos_precalc - 2);
	this->psWin->DrawLineTo(this->psWin->iWidth, this->ypos_precalc - 2);
	PsPoint ptsr[3], * pts = ptsr;
	pts[0].x = 0; pts[0].y = this->ypos_precalc - 2;
	pts[1].x = 0; pts[1].y = this->ypos_precalc - 5;
	pts[2].x = 3; pts[2].y = this->ypos_precalc - 2;
	this->psWin->DrawPolygon(pts, 3);
	pts[0].x = 0; pts[0].y = this->ypos_precalc;
	pts[1].x = 0; pts[1].y = this->ypos_precalc + 3;
	pts[2].x = 3; pts[2].y = this->ypos_precalc;
	this->psWin->DrawPolygon(pts, 3);
	pts[0].x = this->psWin->iWidth - this->s.right - 1; pts[0].y = this->ypos_precalc - 2;
	pts[1].x = this->psWin->iWidth - this->s.right - 1; pts[1].y = this->ypos_precalc - 5;
	pts[2].x = this->psWin->iWidth - this->s.right - 4; pts[2].y = this->ypos_precalc - 2;
	this->psWin->DrawPolygon(pts, 3);
	pts[0].x = this->psWin->iWidth - this->s.right - 1; pts[0].y = this->ypos_precalc;
	pts[1].x = this->psWin->iWidth - this->s.right - 1; pts[1].y = this->ypos_precalc + 3;
	pts[2].x = this->psWin->iWidth - this->s.right - 4; pts[2].y = this->ypos_precalc;
	this->psWin->DrawPolygon(pts, 3);
}
