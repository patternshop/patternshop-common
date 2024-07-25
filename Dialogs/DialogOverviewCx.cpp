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

#include "PsProjectController.h"
#include "DialogOverviewCx.h"

DialogOverviewCx::DialogOverviewCx(PsWin* psWin) : psWin(psWin)
{
	this->bUpdated = false;
	this->bDragging = false;
	this->zooming = false;
	this->border_size = 60;
}

void DialogOverviewCx::Update()
{
	if (this->bUpdated) this->bUpdated = false;
}

PsRect DialogOverviewCx::GetSelectionRectangle(int iWindowWidth, int iWindowHeight)
{
	PsRender& renderer = PsController::Instance().project_controller->renderer;

	float size_x = renderer.size_x;
	float size_y = renderer.size_y;
	float scroll_x = renderer.scroll_x;
	float scroll_y = renderer.scroll_y;
	float x1 = renderer.x1;
	float x2 = renderer.x2;
	float y1 = renderer.y1;
	float y2 = renderer.y2;
	float zoom = renderer.zoom;
	int height = iWindowHeight - 14;

	PsRect z;

	renderer.SetSize(iWindowWidth, height);
	renderer.CenterView();

	if (x1 > 0) z.left = (x1 - renderer.x1) / (renderer.x2 - renderer.x1) * iWindowWidth;
	else z.left = (0 - renderer.x1) / (renderer.x2 - renderer.x1) * iWindowWidth;
	if (x2 < renderer.doc_x) z.right = (x2 - renderer.x1) / (renderer.x2 - renderer.x1) * iWindowWidth;
	else z.right = (renderer.doc_x - renderer.x1) / (renderer.x2 - renderer.x1) * iWindowWidth;

	if (y1 < renderer.doc_y) z.bottom = height - (y1 - renderer.y1) / (renderer.y2 - renderer.y1) * height;
	else z.bottom = height - (renderer.doc_y - renderer.y1) / (renderer.y2 - renderer.y1) * height;
	if (y2 > 0) z.top = height - (y2 - renderer.y1) / (renderer.y2 - renderer.y1) * height;
	else z.top = height - (0 - renderer.y1) / (renderer.y2 - renderer.y1) * height;

	renderer.SetSize(size_x, size_y);
	renderer.SetZoom(zoom);
	renderer.SetScroll(scroll_x, scroll_y);

	return z;
}

void DialogOverviewCx::OnLeftMouseButtonDown(PsPoint point)
{
	if (PsController::Instance().project_controller)
	{
		PsRender& renderer = PsController::Instance().project_controller->renderer;
		int x = FloatToInt((this->window_buffer2.GetWidth() - this->r_size_x) / 2);
		int y = FloatToInt((this->window_buffer2.GetHeight() - this->r_size_y) / 2);
		if (this->bDragging || !this->zooming &&
			point.x > x && point.x < x + this->r_size_x
			&& point.y > y && point.y < y + this->r_size_y)
		{
			float scroll_x = (point.x - x) * this->r_zoom;
			float scroll_y = (point.y - y) * this->r_zoom;
			renderer.SetScroll(scroll_x, scroll_y);
			if (renderer.x2 - renderer.x1 > renderer.doc_x)
			{
				if (renderer.scroll_x != (float)renderer.doc_x / 2.f)
				{
					renderer.scroll_x = (float)renderer.doc_x / 2.f;
					renderer.Recalc();
				}
			}
			if (renderer.y1 - renderer.y2 > renderer.doc_y)
			{
				if (renderer.scroll_y != (float)renderer.doc_y / 2.f)
				{
					renderer.scroll_y = (float)renderer.doc_y / 2.f;
					renderer.Recalc();
				}
			}
			PsController::Instance().UpdateWindow();
			this->bDragging = true;
		}
		else if (this->zooming || !this->bDragging &&
			point.x > this->border_size && point.x < this->window_buffer2.GetWidth() - 20
			&& point.y > this->window_buffer2.GetHeight() - 11)
		{
			float fZoomRange = renderer.fZoomMax - renderer.fZoomMin;
			int iLineWidth = (this->window_buffer2.GetWidth() - this->border_size - 20);
			/*float zoom = (point.x - border_size)
				/ (float)(window_buffer2.GetWidth() - border_size - 20)
				* (renderer.fZoomMax - renderer.fZoomMin) + renderer.fZoomMin;
			zoom = renderer.fZoomMax - zoom;*/
			float zoom = (-fZoomRange * (point.x - this->border_size)) / (float)iLineWidth + fZoomRange + renderer.fZoomMin;
			renderer.zoom = zoom;
			this->zooming = true;
			PsController::Instance().UpdateWindow();
		}
	}
}

void DialogOverviewCx::OnLeftMouseButtonUp(PsPoint p)
{
	if (this->bDragging)
		this->bDragging = false;
	if (this->zooming)
		this->zooming = false;
}

void DialogOverviewCx::CleanBackground()
{
	this->psWin->SetPenColor(212, 208, 200);
	this->psWin->SetBrushColor(212, 208, 200);
	this->psWin->DrawRectangle(0, 0, this->window_buffer2.GetWidth(), this->window_buffer2.GetHeight());
}

void DialogOverviewCx::FastUpdate()
{
	if (!PsController::Instance().project_controller)
	{
		this->psWin->SetTarget(&this->window_buffer);
		this->CleanBackground();
		this->psWin->SetTarget(NULL);
		return;
	}

	this->psWin->SetTarget(&this->window_buffer);

	this->psWin->DrawSoftwareBuffer(this->window_buffer2, 0, 0);

	if (PsController::Instance().project_controller)
	{
		this->DrawRedSelection();

		PsRender& renderer = PsController::Instance().project_controller->renderer;

		// zoom bar
		this->psWin->SetPenColor(0, 0, 0);
		this->psWin->MovePenTo(this->border_size, this->window_buffer2.GetHeight() - 11);
		this->psWin->DrawLineTo(this->window_buffer2.GetWidth() - 20, this->window_buffer2.GetHeight() - 11);
		float fZoomRange = renderer.fZoomMax - renderer.fZoomMin;
		int iLineWidth = (this->window_buffer2.GetWidth() - this->border_size - 20);
		int x = FloatToInt((1 - (renderer.zoom - renderer.fZoomMin) / fZoomRange) * iLineWidth);
		this->psWin->MovePenTo(this->border_size + x, this->window_buffer2.GetHeight() - 10);
		this->psWin->DrawLineTo(this->border_size + x - 5, this->window_buffer2.GetHeight() - 5);
		this->psWin->DrawLineTo(this->border_size + x + 5, this->window_buffer2.GetHeight() - 5);
		this->psWin->DrawLineTo(this->border_size + x, this->window_buffer2.GetHeight() - 10);

		// text version
		this->psWin->SetBrushColor(255, 255, 255);
		this->psWin->DrawRectangle(0, this->window_buffer2.GetHeight() - 14, this->border_size - 5, this->window_buffer2.GetHeight());
		char buffer_t[1024];
		sprintf(buffer_t, "%.2f%%", (1.f / renderer.zoom) * 100.f);
		PsRect p;
		p.left = 5; p.top = this->window_buffer2.GetHeight() - 15;
		p.right = this->window_buffer2.GetWidth(); p.bottom = this->window_buffer2.GetHeight();
		this->psWin->SetTextColor(0, 0, 0);
		this->psWin->DrawText(buffer_t, p, PsWin::PSFONT_NORMAL);
	}

	this->psWin->SetTarget(NULL);
}
