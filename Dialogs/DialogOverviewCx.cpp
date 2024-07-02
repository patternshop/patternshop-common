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

#include "PsProject.h"
#include "PsWinOverview.h"

DialogOverviewCx::DialogOverviewCx()
{
	bUpdated = false;
	bDragging = false;
	zooming = false;
	border_size = 60;
}

PsRect DialogOverviewCx::GetSelectionRectangle(int iWindowWidth, int iWindowHeight)
{
	PsRender& renderer = PsController::Instance().project->renderer;

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
	if (PsController::Instance().project)
	{
		PsRender& renderer = PsController::Instance().project->renderer;
		int x = FloatToInt((window_buffer2.GetWidth() - r_size_x) / 2);
		int y = FloatToInt((window_buffer2.GetHeight() - r_size_y) / 2);
		if (bDragging || !zooming &&
			point.x > x && point.x < x + r_size_x
			&& point.y > y && point.y < y + r_size_y)
		{
			float scroll_x = (point.x - x) * r_zoom;
			float scroll_y = (point.y - y) * r_zoom;
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
			bDragging = true;
		}
		else if (zooming || !bDragging &&
			point.x > border_size && point.x < window_buffer2.GetWidth() - 20
			&& point.y > window_buffer2.GetHeight() - 11)
		{
			float fZoomRange = renderer.fZoomMax - renderer.fZoomMin;
			int iLineWidth = (window_buffer2.GetWidth() - border_size - 20);
			/*float zoom = (point.x - border_size)
				/ (float)(window_buffer2.GetWidth() - border_size - 20)
				* (renderer.fZoomMax - renderer.fZoomMin) + renderer.fZoomMin;
			zoom = renderer.fZoomMax - zoom;*/
			float zoom = (-fZoomRange * (point.x - border_size)) / (float)iLineWidth + fZoomRange + renderer.fZoomMin;
			renderer.zoom = zoom;
			zooming = true;
			PsController::Instance().UpdateWindow();
		}
	}
}

void DialogOverviewCx::OnLeftMouseButtonUp(PsPoint p)
{
	if (bDragging)
		bDragging = false;
	if (zooming)
		zooming = false;
}

void PsWinOverview::UpdateMiniImage()
{
	PsRender& renderer = PsController::Instance().project->renderer;

	float size_x = renderer.size_x;
	float size_y = renderer.size_y;
	float scroll_x = renderer.scroll_x;
	float scroll_y = renderer.scroll_y;
	/*float x1 = renderer.x1;
	float x2 = renderer.x2;
	float y1 = renderer.y1;
	float y2 = renderer.y2;*/
	float zoom = renderer.zoom;
	bool showbox = PsController::Instance().GetOption(PsController::OPTION_BOX_SHOW);
	bool showmat = PsController::Instance().GetOption(PsController::OPTION_HIGHLIGHT_SHOW);
	int height = window_buffer2.GetHeight() - 14;

	PsController::Instance().SetOption(PsController::OPTION_BOX_SHOW, false);
	PsController::Instance().SetOption(PsController::OPTION_HIGHLIGHT_SHOW, false);
	renderer.SetSize(window_buffer2.GetWidth(), height);
	renderer.CenterView();

	PsController::Instance().project->RenderToScreen();
	hardwareRenderer.CopyToSoftBuffer(m_RenduImage);

	PsRect z; // calcul de la zone de l'image
	z.left = (0 - renderer.x1) / (renderer.x2 - renderer.x1) * window_buffer2.GetWidth();
	z.right = (renderer.doc_x - renderer.x1) / (renderer.x2 - renderer.x1) * window_buffer2.GetWidth();
	z.bottom = height - (renderer.doc_y - renderer.y1) / (renderer.y2 - renderer.y1) * height;
	z.top = height - (0 - renderer.y1) / (renderer.y2 - renderer.y1) * height;
	r_size_x = z.right - z.left;
	r_size_y = z.bottom - z.top;
	r_zoom = renderer.zoom;

	SetTarget(&window_buffer2);
	CleanBackground();
	SetTarget(NULL);

	int x = FloatToInt((m_RenduImage.GetWidth() - r_size_x) / 2);
	int y = FloatToInt((height - r_size_y) / 2);

#ifdef _WINDOWS
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SelectObject(window_buffer2.buffer);
	CDC dc2;
	dc2.CreateCompatibleDC(NULL);
	dc2.SelectObject(m_RenduImage.buffer);
	dc.BitBlt(x, y, r_size_x, r_size_y, &dc2, z.left, m_RenduImage.buffer.GetHeight() - height + z.top, SRCCOPY);
	dc2.DeleteDC();
#else /* _MACOSX */
	SetTarget(&window_buffer2);
	CGRect subRect = CGRectMake(z.left, m_RenduImage.GetHeight() - height + z.top, r_size_x, r_size_y);
	CGImageRef subImage = CGImageCreateWithImageInRect(m_RenduImage.buffer, subRect);
	SoftwareBuffer sb;
	sb.buffer = subImage;
	DrawSoftwareBuffer(sb, x, y);
	SetTarget(NULL);
#endif

	PsRect r;
	r.left = x - 1;
	r.top = y - 1;
	r.right = x + r_size_x + 1;
	r.bottom = y + r_size_y + 1;

#ifdef _WINDOWS
	dc.DrawEdge(&r, EDGE_SUNKEN, BF_RECT);
#else /* _MACOSX */
	/*
	SetPenColor(0, 0, 0);
	MovePenTo(r.left + 1, r.top + 2);
	DrawLineTo(r.right, r.top + 2);
	DrawLineTo(r.right, r.bottom + 1);
	DrawLineTo(r.left + 1, r.bottom + 1);
	DrawLineTo(r.left + 1, r.top + 2);
	 */
#endif

	PsController::Instance().SetOption(PsController::OPTION_HIGHLIGHT_SHOW, showmat);
	PsController::Instance().SetOption(PsController::OPTION_BOX_SHOW, showbox);
	renderer.SetSize(size_x, size_y);
	renderer.SetZoom(zoom);
	renderer.SetScroll(scroll_x, scroll_y);

#ifdef _MACOSX
	//PsController::Instance().project->RenderToScreen(); // trouver mieux en direct rendering...
#endif
}

void PsWinOverview::CleanBackground()
{
	SetPenColor(212, 208, 200);
	SetBrushColor(212, 208, 200);
	DrawRectangle(0, 0, window_buffer2.GetWidth(), window_buffer2.GetHeight());
}

void PsWinOverview::FastUpdate()
{
	if (!PsController::Instance().project)
	{
		SetTarget(&window_buffer);
		CleanBackground();
		SetTarget(NULL);
		return;
	}

	SetTarget(&window_buffer);


	DrawSoftwareBuffer(window_buffer2, 0, 0);

	if (PsController::Instance().project)
	{
		DrawRedSelection();

		PsRender& renderer = PsController::Instance().project->renderer;

		// barre de zoom
		SetPenColor(0, 0, 0);
		MovePenTo(border_size, window_buffer2.GetHeight() - 11);
		DrawLineTo(window_buffer2.GetWidth() - 20, window_buffer2.GetHeight() - 11);
		float fZoomRange = renderer.fZoomMax - renderer.fZoomMin;
		int iLineWidth = (window_buffer2.GetWidth() - border_size - 20);
		int x = FloatToInt((1 - (renderer.zoom - renderer.fZoomMin) / fZoomRange) * iLineWidth);
		MovePenTo(border_size + x, window_buffer2.GetHeight() - 10);
		DrawLineTo(border_size + x - 5, window_buffer2.GetHeight() - 5);
		DrawLineTo(border_size + x + 5, window_buffer2.GetHeight() - 5);
		DrawLineTo(border_size + x, window_buffer2.GetHeight() - 10);

		// version texte
		SetBrushColor(255, 255, 255);
		DrawRectangle(0, window_buffer2.GetHeight() - 14, border_size - 5, window_buffer2.GetHeight());
		char buffer_t[1024];
		sprintf(buffer_t, "%.2f%%", (1.f / renderer.zoom) * 100.f);
		PsRect p;
		p.left = 5; p.top = window_buffer2.GetHeight() - 15;
		p.right = window_buffer2.GetWidth(); p.bottom = window_buffer2.GetHeight();
		SetTextColor(0, 0, 0);
		DrawText(buffer_t, p, PSFONT_NORMAL);
	}

	SetTarget(NULL);
}
