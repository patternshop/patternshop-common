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
	if (ypos_precalc + bloc_count_size > 0)
	{
		psWin->SetBrushColor(212, 208, 200);
		psWin->SetPenColor(255, 255, 255);
		bool close = true;
		if (openCloseMap[matrix] != CLOSE)
		{
			psWin->DrawRectangle(0, ypos_precalc, 25, ypos_precalc + bloc_count_size
				+ item_count_size * (int)matrix->images.size() - 1);
			close = false;
		}
		else psWin->DrawRectangle(0, ypos_precalc, 25, ypos_precalc + bloc_count_size - 1);

		if (!matrix->hide) viewImage.BitBlt((*psWin), 4, ypos_precalc + 2);
		else boxImage.BitBlt((*psWin), 4, ypos_precalc + 2);

		if (selected != matrix)
		{
			psWin->SetBrushColor(212, 208, 200);
			psWin->SetPenColor(212, 208, 200);;
			psWin->DrawRectangle(26, ypos_precalc, psWin->iWidth, ypos_precalc + bloc_count_size);
			if (!close) openImage.BitBlt((*psWin), 27, ypos_precalc + 8);
			else closeImage.BitBlt((*psWin), 27, ypos_precalc + 5);
			directoryImage.BitBlt((*psWin), 45, ypos_precalc + 4);
		}
		else
		{
			psWin->SetBrushColor(10, 36, 106);
			psWin->SetPenColor(10, 36, 106);
			psWin->DrawRectangle(26, ypos_precalc, psWin->iWidth, ypos_precalc + bloc_count_size);
			if (!close) openImageB.BitBlt((*psWin), 27, ypos_precalc + 8);
			else closeImageB.BitBlt((*psWin), 27, ypos_precalc + 5);
			directoryImageB.BitBlt((*psWin), 45, ypos_precalc + 4);
		}
		//
		PsRect p;
		char buffer[1024];
		sprintf(buffer, "%s %d", GetLabel(LABEL_MATRIX), matNameCount);
		p.left = 65; p.top = ypos_precalc;
		p.right = psWin->iWidth; p.bottom = ypos_precalc + bloc_count_size;
		PsWin::PsFont MyFont = PsWin::PSFONT_NORMAL;
		if (selected != matrix)
		{
			psWin->SetTextColor(0, 0, 0);
		}
		else
		{
			psWin->SetTextColor(255, 255, 255);
			MyFont = PsWin::PSFONT_BOLD;
		}
		psWin->DrawText(buffer, p, MyFont);
		//
		psWin->SetPenColor(255, 255, 255);
		psWin->MovePenTo(28, ypos_precalc + bloc_count_size);
		psWin->DrawLineTo(psWin->iWidth, ypos_precalc + bloc_count_size);
	}
	bloc_count++;
	matNameCount++;
	ypos_precalc += bloc_count_size;
}

void PsWinProjectCx::DrawImageBloc(PsImage* image)
{
	if (ypos_precalc + item_count_size > 0)
	{
		int left_space_pixels = 5;
		if (image->parent) left_space_pixels = 15;

		if (!image->hide) viewImage.BitBlt((*psWin), 4, ypos_precalc + 4);
		else boxImage.BitBlt((*psWin), 4, ypos_precalc + 4);

		psWin->SetBrushColor(255, 255, 255);
		psWin->SetPenColor(255, 255, 255);
		psWin->DrawRectangle(26, ypos_precalc, psWin->iWidth, ypos_precalc + item_count_size);
		if (selected == image)
		{
			psWin->SetBrushColor(10, 36, 106);
			psWin->SetPenColor(10, 36, 106);
			psWin->DrawRectangle(26, ypos_precalc, psWin->iWidth, ypos_precalc + item_count_size - 1);
		}

		SoftwareBuffer* img = imageList[image->GetAutoGenId()];
		if (!img) img = loadThumb(&image->GetTexture());
		assert(img);
		int w = img->GetWidth();
		int h = img->GetHeight();
		int p = (32 - w) / 2 + left_space_pixels + 26;
		int u = (item_count_size - h) / 2 + ypos_precalc;
		img->BitBlt((*psWin), p, u);
		if (selected == image)
		{
			psWin->SetPenColor(10, 36, 106);
			psWin->MovePenTo(p, u);
			psWin->DrawLineTo(p + w - 1, u);
			psWin->DrawLineTo(p + w - 1, u + h - 1);
			psWin->DrawLineTo(p, u + h - 1);
			psWin->DrawLineTo(p, u);
			psWin->SetPenColor(255, 255, 255);
			psWin->MovePenTo(p - 1, u - 1);
			psWin->DrawLineTo(p + w, u - 1);
			psWin->DrawLineTo(p + w, u + h);
			psWin->DrawLineTo(p - 1, u + h);
			psWin->DrawLineTo(p - 1, u - 1);
		}
		else
		{
			psWin->SetPenColor(0, 0, 0);
			psWin->MovePenTo(p, u);
			psWin->DrawLineTo(p + w - 1, u);
			psWin->DrawLineTo(p + w - 1, u + h - 1);
			psWin->DrawLineTo(p, u + h - 1);
			psWin->DrawLineTo(p, u);
		}

		PsRect t;
		char buffer[1024];
		if (image->parent) sprintf(buffer, "%s %d", GetLabel(LABEL_IMAGE), motifNameCount);
		else sprintf(buffer, "%s %d", GetLabel(LABEL_IMAGE), imageNameCount);
		t.left = 65 + left_space_pixels; t.top = ypos_precalc;
		t.right = psWin->iWidth; t.bottom = ypos_precalc + item_count_size;
		PsWin::PsFont MyFont = PsWin::PSFONT_NORMAL;
		if (selected != image)
		{
			psWin->SetTextColor(0, 0, 0);
		}
		else
		{
			psWin->SetTextColor(255, 255, 255);
			MyFont = PsWin::PSFONT_BOLD;
		}
		psWin->DrawText(buffer, t, MyFont);
		//
		if (bDragging && image != selected
			&& dynamic_cast<PsImage*>(selected)
			&& draggingPoint.y < ypos_precalc + item_count_size
			&& draggingPoint.y > ypos_precalc)
		{
			DrawInsertionCaret();
			dragBefore = image;
		}
	}
	ypos_precalc += item_count_size;
}

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

