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
#ifdef _WINDOWS
# include "stdafx.h"
# include <atlimage.h>
# include "Patternshop.h"
#endif

#ifdef _MACOSX
# include "PsMacView.h"
#endif

#include "PsRender.h"
#include "PsController.h"
#include "PsTexture.h"
#include "PsProjectController.h"
#include "PsSoftRender.h"
#include "PsHardware.h"
#include "PsMaths.h"
#include "PsTypes.h"

/*
 ** When the renderer is created, the background image (the white/gray grid) is loaded.
 */
PsRender::PsRender() :
	doc_x(100),
	doc_y(100),
	scroll_x(0),
	scroll_y(0),
	size_x(0),
	size_y(0),
	zoom(1),
	dpi(300)
{
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.f, 1.f, 1.f, 0.0f);
	glClearDepth(1.f);

	this->fZoomMax = 10.0f;
	this->fZoomMin = 0.05f;

	this->iLayerTextureSize = 512;

#ifdef _MACOSX
	CFStringRef name;
	CFURLRef url;
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	name = CFStringCreateWithCString(NULL, "back", kCFStringEncodingUTF8);
	url = CFBundleCopyResourceURL(mainBundle, name, CFSTR("png"), NULL);
	CFRelease(name);
	assert(url != NULL);
	CFStringRef path = CFURLCopyPath(url);
	char BufferName[1024];
	CFStringGetCString(path, BufferName, 1023, kCFStringEncodingUTF8);
	this->back.LoadFromFile(BufferName);
#else
	CImage b;
	b.LoadFromResource(AfxGetInstanceHandle(), IDB_MOTIF);
	BITMAP	bitmap;
	GetObject(b, sizeof(bitmap), &bitmap);
	this->back.Register(16, 16, 3, (uint8*)bitmap.bmBits);
#endif
}

PsRender::~PsRender()
{

}

/*
 ** Change the scrolling and zoom to fit the entire document into the window.
 */
void PsRender::CenterView()
{
	float	zx;
	float	zy;

	zx = (float)this->doc_x / (float)this->size_x;
	zy = (float)this->doc_y / (float)this->size_y;

	this->SetScroll((float)this->doc_x / 2.0f, (float)this->doc_y / 2.0f);
	this->zoom = (zx < zy ? zy : zx) * RENDERER_ZOOM_INIT;

	this->Recalc();
}

/*
 ** Transforms coordinates in the window (from an OnMouseClick or other) into
 ** document coordinates (which vary according to zoom, scrolling, etc., and are
 ** in float). These latter coordinates are used absolutely everywhere else.
 */
void PsRender::Convert(int x, int y, float& fx, float& fy) const
{
	fx = this->x1 + (this->x2 - this->x1) * (float)x / (float)this->size_x;
	fy = this->y2 + (this->y1 - this->y2) * (float)y / (float)this->size_y;
}

/*
 ** Draws the background, with the grid (hardware only).
 */
void PsRender::DrawBack(const PsProjectController& p, float x, float y)
{

	if (p.bHideColor)
	{
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, this->back.GetID());
		glEnable(GL_TEXTURE_2D);
	}
	else
	{
		glColor3f(p.iColor[0] / 255.f,
			p.iColor[1] / 255.f,
			p.iColor[2] / 255.f);
	}

	glBegin(GL_QUADS);
	glTexCoord2f(0, y * RENDERER_BACKGROUND);
	glVertex2f(this->x1, this->y1);
	glTexCoord2f(0, 0);
	glVertex2f(this->x1, this->y2);
	glTexCoord2f(x * RENDERER_BACKGROUND, 0);
	glVertex2f(this->x2, this->y2);
	glTexCoord2f(x * RENDERER_BACKGROUND, y * RENDERER_BACKGROUND);
	glVertex2f(this->x2, this->y1);
	glEnd();

	if (p.bHideColor)
	{
		glDisable(GL_TEXTURE_2D);
	}
}

/*
 ** Draws a selection box around an image (hardware only).
 */
void PsRender::DrawBox(const PsImage& image)
{
	float corner[4][2];
	int i;

	glColor4f(0, 0, 0, 0.5f);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	image.ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	image.ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	image.ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	image.ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

	float cx = (corner[0][0] + corner[2][0]) / 2.f;
	float cy = (corner[0][1] + corner[2][1]) / 2.f;
	float dim = 3.f * this->zoom;
	glBegin(GL_LINES);
	glVertex2f(cx - dim, cy - dim);
	glVertex2f(cx + dim, cy + dim);
	glVertex2f(cx - dim, cy + dim);
	glVertex2f(cx + dim, cy - dim);
	glEnd();

	//glEnable(GL_LINE_STIPPLE);
	//glLineStipple(1, 0x0F0F); 

	glBegin(GL_LINE_LOOP);
	glVertex2f(corner[0][0], corner[0][1]);
	glVertex2f(corner[1][0], corner[1][1]);
	glVertex2f(corner[2][0], corner[2][1]);
	glVertex2f(corner[3][0], corner[3][1]);
	glEnd();

	//glDisable(GL_LINE_STIPPLE);

	for (i = 0; i < 4; ++i)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] - SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] - SHAPE_SIZE_RESIZE * this->zoom);
		glEnd();
	}

	glDisable(GL_BLEND);

}

void PsRender::DrawBoxHandle(const PsMatrix& matrix)
{
	float corner[4][2];
	int i;
	float	r, g, b;

	matrix.GetColor(r, g, b);
	glColor3f(r, g, b);

	matrix.ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	matrix.ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	matrix.ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	matrix.ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

	/*glBegin(GL_LINE_LOOP);
	glVertex2f(corner[0][0], corner[0][1]);
	glVertex2f(corner[1][0], corner[1][1]);
	glVertex2f(corner[2][0], corner[2][1]);
	glVertex2f(corner[3][0], corner[3][1]);
	glEnd();
	*/

	for (i = 0; i < 4; ++i)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] - SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * this->zoom, corner[i][1] - SHAPE_SIZE_RESIZE * this->zoom);
		glEnd();
	}

	matrix.ToAbsolute(0, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	matrix.ToAbsolute(SHAPE_SIZE, 0, corner[1][0], corner[1][1]);
	matrix.ToAbsolute(0, SHAPE_SIZE, corner[2][0], corner[2][1]);
	matrix.ToAbsolute(-SHAPE_SIZE, 0, corner[3][0], corner[3][1]);

	for (i = 0; i < 4; ++i)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(corner[i][0], corner[i][1] - SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * this->zoom, corner[i][1]);
		glVertex2f(corner[i][0], corner[i][1] + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * this->zoom, corner[i][1]);
		glEnd();
	}
}

/*
 ** Draws a selection box around a matrix (hardware only).
 */
void PsRender::DrawBox(const PsMatrix& matrix)
{
	float corner[4][2];
	float r;
	float g;
	float b;
	int i;

	matrix.GetColor(r, g, b);

	matrix.ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	matrix.ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	matrix.ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	matrix.ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

	if (PsController::Instance().GetOption(PsController::OPTION_HIGHLIGHT_SHOW))
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(r, g, b, 0.25f);

		glBegin(GL_QUADS);
		glVertex2f(corner[0][0], corner[0][1]);
		glVertex2f(corner[1][0], corner[1][1]);
		glVertex2f(corner[2][0], corner[2][1]);
		glVertex2f(corner[3][0], corner[3][1]);
		glEnd();

		glDisable(GL_BLEND);
	}

	if (matrix.div_is_active)
	{
		float p[4];
		float v[4];
		v[0] = (corner[1][0] - corner[0][0]) / (float)matrix.div_x;
		v[1] = (corner[1][1] - corner[0][1]) / (float)matrix.div_x;
		v[2] = (corner[2][0] - corner[3][0]) / (float)matrix.div_x;
		v[3] = (corner[2][1] - corner[3][1]) / (float)matrix.div_x;
		for (i = 0; i < matrix.div_x; ++i)
		{
			p[0] = corner[0][0] + v[0] * (i + 1.f);
			p[1] = corner[0][1] + v[1] * (i + 1.f);
			p[2] = corner[3][0] + v[2] * (i + 1.f);
			p[3] = corner[3][1] + v[3] * (i + 1.f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(p[0], p[1]);
			glVertex2f(p[2], p[3]);
			glEnd();
		}
		v[0] = (corner[3][0] - corner[0][0]) / (float)matrix.div_y;
		v[1] = (corner[3][1] - corner[0][1]) / (float)matrix.div_y;
		v[2] = (corner[2][0] - corner[1][0]) / (float)matrix.div_y;
		v[3] = (corner[2][1] - corner[1][1]) / (float)matrix.div_y;
		for (i = 0; i < matrix.div_y; ++i)
		{
			p[0] = corner[0][0] + v[0] * (i + 1.f);
			p[1] = corner[0][1] + v[1] * (i + 1.f);
			p[2] = corner[1][0] + v[2] * (i + 1.f);
			p[3] = corner[1][1] + v[3] * (i + 1.f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(p[0], p[1]);
			glVertex2f(p[2], p[3]);
			glEnd();
		}
	}

	glColor3f(r, g, b);

	glBegin(GL_LINE_LOOP);
	glVertex2f(corner[0][0], corner[0][1]);
	glVertex2f(corner[1][0], corner[1][1]);
	glVertex2f(corner[2][0], corner[2][1]);
	glVertex2f(corner[3][0], corner[3][1]);
	glEnd();

	glColor4f(1.f, 1.f, 1.f, 1.f);
}

/*
 ** Draws the document borders (hardware only).
 */
void PsRender::DrawDocument()
{
	if (PsController::Instance().GetOption(PsController::OPTION_DOCUMENT_BLEND))
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.75f, 0.75f, 0.75f, 0.75f);
	}
	else
		glColor3f(0.75f, 0.75f, 0.75f);

	glBegin(GL_QUADS);
	glVertex2f(0, this->y1);
	glVertex2f(0, this->y2);
	glVertex2f(this->x1, this->y2);
	glVertex2f(this->x1, this->y1);

	glVertex2f(this->x2, this->y1);
	glVertex2f(this->x2, this->y2);
	glVertex2f((float)this->doc_x, this->y2);
	glVertex2f((float)this->doc_x, this->y1);

	glVertex2f(0, 0);
	glVertex2f((float)this->doc_x, 0);
	glVertex2f((float)this->doc_x, this->y2);
	glVertex2f(0, this->y2);

	glVertex2f(0, this->y1);
	glVertex2f((float)this->doc_x, this->y1);
	glVertex2f((float)this->doc_x, (float)this->doc_y);
	glVertex2f(0, (float)this->doc_y);
	glEnd();

	glDisable(GL_BLEND);
	glColor3f(0.25f, 0.25f, 0.25f);

	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0f, 0.0f);
	glVertex2f((float)this->doc_x, 0.0f);
	glVertex2f((float)this->doc_x, (float)this->doc_y);
	glVertex2f(0.0f, (float)this->doc_y);
	glEnd();
}

/*
 ** Draws a pattern, either in normal mode ("behind" the scene, and in positive), or in mask mode ("in front" of the scene, in negative).
 */
void PsRender::DrawPattern(PsPattern& pattern/*, bool mask*/)
{
	if (this->engine == ENGINE_HARDWARE)
	{
		glColor4f(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		// background
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, pattern.texture.GetID());
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, 0);
		glTexCoord2f(1, 1);
		glVertex2f((float)this->doc_x, 0);
		glTexCoord2f(1, 0);
		glVertex2f((float)this->doc_x, (float)this->doc_y);
		glTexCoord2f(0, 0);
		glVertex2f(0, (float)this->doc_y);
		glEnd();

		// shadow  
		glColor4f(0, 0, 0, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, pattern.y_map_texture_id);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, 0);
		glTexCoord2f(1, 1);
		glVertex2f((float)this->doc_x, 0);
		glTexCoord2f(1, 0);
		glVertex2f((float)this->doc_x, (float)this->doc_y);
		glTexCoord2f(0, 0);
		glVertex2f(0, (float)this->doc_y);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	else
	{
		PasteSoftwareFile(pattern);
	}
}

/*
 ** Draws an image at the absolute position x, y. The boolean "precalc" indicates whether or not to calculate the coordinates of the image corners (to be added to x and y), since they remain the same for an image, it is unnecessary to recalculate them each time it is displayed, only x and y change.
 */
void PsRender::DrawShape(PsImage& image, float x, float y, bool bFirst, bool bLast)
{
	if (image.hide)
		return;

	if (bFirst)
	{
		image.ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, image.corner[0][0], image.corner[0][1]);
		image.ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, image.corner[1][0], image.corner[1][1]);
		image.ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, image.corner[2][0], image.corner[2][1]);
		image.ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, image.corner[3][0], image.corner[3][1]);
	}

	if (this->engine == ENGINE_HARDWARE)
	{
		if (bFirst)
		{
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBindTexture(GL_TEXTURE_2D, image.texture.GetID());
		}

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(x + image.corner[0][0], y + image.corner[0][1]);
		glTexCoord2f(1, 1);
		glVertex2f(x + image.corner[1][0], y + image.corner[1][1]);
		glTexCoord2f(1, 0);
		glVertex2f(x + image.corner[2][0], y + image.corner[2][1]);
		glTexCoord2f(0, 0);
		glVertex2f(x + image.corner[3][0], y + image.corner[3][1]);
		glEnd();

		if (bLast)
		{
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
	else
	{
		PasteSoftwareFile(image, (int)x, (int)y);
	}
}

