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

void PsWinProjectCx::DrawMatrixBloc(PsMatrix* matrix)
{
	if (this->ypos_precalc + this->bloc_count_size > 0)
	{
		this->psWin->SetBrushColor(212, 208, 200);
		this->psWin->SetPenColor(255, 255, 255);
		bool close = true;
		if (this->openCloseMap[matrix] != CLOSE)
		{
			this->psWin->DrawRectangle(0, this->ypos_precalc, 25, this->ypos_precalc + this->bloc_count_size
				+ this->item_count_size * (int)matrix->images.size() - 1);
			close = false;
		}
		else this->psWin->DrawRectangle(0, this->ypos_precalc, 25, this->ypos_precalc + this->bloc_count_size - 1);

		if (!matrix->hide) this->viewImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 2);
		else this->boxImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 2);

		if (this->selected != matrix)
		{
			this->psWin->SetBrushColor(212, 208, 200);
			this->psWin->SetPenColor(212, 208, 200);;
			this->psWin->DrawRectangle(26, this->ypos_precalc, this->psWin->iWidth, this->ypos_precalc + this->bloc_count_size);
			if (!close) this->openImage.BitBlt((*this->psWin), 27, this->ypos_precalc + 8);
			else this->closeImage.BitBlt((*this->psWin), 27, this->ypos_precalc + 5);
			this->directoryImage.BitBlt((*this->psWin), 45, this->ypos_precalc + 4);
		}
		else
		{
			this->psWin->SetBrushColor(10, 36, 106);
			this->psWin->SetPenColor(10, 36, 106);
			this->psWin->DrawRectangle(26, this->ypos_precalc, this->psWin->iWidth, this->ypos_precalc + this->bloc_count_size);
			if (!close) this->openImageB.BitBlt((*this->psWin), 27, this->ypos_precalc + 8);
			else this->closeImageB.BitBlt((*this->psWin), 27, this->ypos_precalc + 5);
			this->directoryImageB.BitBlt((*this->psWin), 45, this->ypos_precalc + 4);
		}
		//
		PsRect p;
		char buffer[1024];
		sprintf(buffer, "%s %d", GetLabel(LABEL_MATRIX), this->matNameCount);
		p.left = 65; p.top = this->ypos_precalc;
		p.right = this->psWin->iWidth; p.bottom = this->ypos_precalc + this->bloc_count_size;
		PsWin::PsFont MyFont = PsWin::PSFONT_NORMAL;
		if (this->selected != matrix)
		{
			this->psWin->SetTextColor(0, 0, 0);
		}
		else
		{
			this->psWin->SetTextColor(255, 255, 255);
			MyFont = PsWin::PSFONT_BOLD;
		}
		this->psWin->DrawText(buffer, p, MyFont);
		//
		this->psWin->SetPenColor(255, 255, 255);
		this->psWin->MovePenTo(28, this->ypos_precalc + this->bloc_count_size);
		this->psWin->DrawLineTo(this->psWin->iWidth, this->ypos_precalc + this->bloc_count_size);
	}
	this->bloc_count++;
	this->matNameCount++;
	this->ypos_precalc += this->bloc_count_size;
}

void PsWinProjectCx::DrawImageBloc(PsImage* image)
{
	if (this->ypos_precalc + this->item_count_size > 0)
	{
		int left_space_pixels = 5;
		if (image->parent) left_space_pixels = 15;

		if (!image->hide) this->viewImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 4);
		else this->boxImage.BitBlt((*this->psWin), 4, this->ypos_precalc + 4);

		this->psWin->SetBrushColor(255, 255, 255);
		this->psWin->SetPenColor(255, 255, 255);
		this->psWin->DrawRectangle(26, this->ypos_precalc, this->psWin->iWidth, this->ypos_precalc + this->item_count_size);
		if (this->selected == image)
		{
			this->psWin->SetBrushColor(10, 36, 106);
			this->psWin->SetPenColor(10, 36, 106);
			this->psWin->DrawRectangle(26, this->ypos_precalc, this->psWin->iWidth, this->ypos_precalc + this->item_count_size - 1);
		}

		SoftwareBuffer* img = this->imageList[image->GetAutoGenId()];
		if (!img) img = this->loadThumb(&image->GetTexture());
		assert(img);
		int w = img->GetWidth();
		int h = img->GetHeight();
		int p = (32 - w) / 2 + left_space_pixels + 26;
		int u = (this->item_count_size - h) / 2 + this->ypos_precalc;
		img->BitBlt((*this->psWin), p, u);
		if (this->selected == image)
		{
			this->psWin->SetPenColor(10, 36, 106);
			this->psWin->MovePenTo(p, u);
			this->psWin->DrawLineTo(p + w - 1, u);
			this->psWin->DrawLineTo(p + w - 1, u + h - 1);
			this->psWin->DrawLineTo(p, u + h - 1);
			this->psWin->DrawLineTo(p, u);
			this->psWin->SetPenColor(255, 255, 255);
			this->psWin->MovePenTo(p - 1, u - 1);
			this->psWin->DrawLineTo(p + w, u - 1);
			this->psWin->DrawLineTo(p + w, u + h);
			this->psWin->DrawLineTo(p - 1, u + h);
			this->psWin->DrawLineTo(p - 1, u - 1);
		}
		else
		{
			this->psWin->SetPenColor(0, 0, 0);
			this->psWin->MovePenTo(p, u);
			this->psWin->DrawLineTo(p + w - 1, u);
			this->psWin->DrawLineTo(p + w - 1, u + h - 1);
			this->psWin->DrawLineTo(p, u + h - 1);
			this->psWin->DrawLineTo(p, u);
		}

		PsRect t;
		char buffer[1024];
		if (image->parent) sprintf(buffer, "%s %d", GetLabel(LABEL_IMAGE), this->motifNameCount);
		else sprintf(buffer, "%s %d", GetLabel(LABEL_IMAGE), this->imageNameCount);
		t.left = 65 + left_space_pixels; t.top = this->ypos_precalc;
		t.right = this->psWin->iWidth; t.bottom = this->ypos_precalc + this->item_count_size;
		PsWin::PsFont MyFont = PsWin::PSFONT_NORMAL;
		if (this->selected != image)
		{
			this->psWin->SetTextColor(0, 0, 0);
		}
		else
		{
			this->psWin->SetTextColor(255, 255, 255);
			MyFont = PsWin::PSFONT_BOLD;
		}
		this->psWin->DrawText(buffer, t, MyFont);
		//
		if (this->bDragging && image != this->selected
			&& dynamic_cast<PsImage*>(this->selected)
			&& this->draggingPoint.y < this->ypos_precalc + this->item_count_size
			&& this->draggingPoint.y > this->ypos_precalc)
		{
			this->DrawInsertionCaret();
			this->dragBefore = image;
		}
	}
	this->ypos_precalc += this->item_count_size;
}
