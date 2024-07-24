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
#include "PsProject.h"
#include "PsSoftRender.h"
#include "PsHardware.h"
#include "PsMaths.h"
#include "PsTypes.h"

/*
 ** À la création du renderer, on charge l'image de fond (le quadriage blanc/gris).
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

	fZoomMax = 10.0f;
	fZoomMin = 0.05f;

	iLayerTextureSize = 512;

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
	back.LoadFromFile(BufferName);
#else
	CImage b;
	b.LoadFromResource(AfxGetInstanceHandle(), IDB_MOTIF);
	BITMAP	bitmap;
	GetObject(b, sizeof(bitmap), &bitmap);
	back.Register(16, 16, 3, (uint8*)bitmap.bmBits);
#endif
}

PsRender::~PsRender()
{

}

/*
 ** Change le scrolling et le zoom pour faire entrer le document entier dans la fenêtre.
 */
void PsRender::CenterView()
{
	float	zx;
	float	zy;

	zx = (float)doc_x / (float)size_x;
	zy = (float)doc_y / (float)size_y;

	SetScroll((float)doc_x / 2.0f, (float)doc_y / 2.0f);
	zoom = (zx < zy ? zy : zx) * RENDERER_ZOOM_INIT;

	Recalc();
}

/*
 ** Transforme des coordonnées dans la fenêtre(donc issues d'un OnMouseClick ou autre) en
 ** coordonnées dans le document(qui varient donc selon le zoom, le scrolling, etc, et qui
								 ** sont en float). Ce sont ces dernières coordonnées qui sont utilisées absolument partout
 ** ailleurs.
 */
void PsRender::Convert(int x, int y, float& fx, float& fy) const
{
	fx = x1 + (x2 - x1) * (float)x / (float)size_x;
	fy = y2 + (y1 - y2) * (float)y / (float)size_y;
}

/*
 ** Dessine l'arrière plan, avec le quadrillage(uniquement hardware).
 */
void PsRender::DrawBack(const PsProject& p, float x, float y)
{

	if (p.bHideColor)
	{
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, back.GetID());
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
	glVertex2f(x1, y1);
	glTexCoord2f(0, 0);
	glVertex2f(x1, y2);
	glTexCoord2f(x * RENDERER_BACKGROUND, 0);
	glVertex2f(x2, y2);
	glTexCoord2f(x * RENDERER_BACKGROUND, y * RENDERER_BACKGROUND);
	glVertex2f(x2, y1);
	glEnd();

	if (p.bHideColor)
	{
		glDisable(GL_TEXTURE_2D);
	}
}

/*
 ** Dessine une boite de sélection autour d'une image(uniquement hardware).
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
	float dim = 3.f * zoom;
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
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * zoom, corner[i][1] - SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * zoom, corner[i][1] + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * zoom, corner[i][1] + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * zoom, corner[i][1] - SHAPE_SIZE_RESIZE * zoom);
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
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * zoom, corner[i][1] - SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * zoom, corner[i][1] + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * zoom, corner[i][1] + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * zoom, corner[i][1] - SHAPE_SIZE_RESIZE * zoom);
		glEnd();
	}

	matrix.ToAbsolute(0, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	matrix.ToAbsolute(SHAPE_SIZE, 0, corner[1][0], corner[1][1]);
	matrix.ToAbsolute(0, SHAPE_SIZE, corner[2][0], corner[2][1]);
	matrix.ToAbsolute(-SHAPE_SIZE, 0, corner[3][0], corner[3][1]);

	for (i = 0; i < 4; ++i)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(corner[i][0], corner[i][1] - SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] - SHAPE_SIZE_RESIZE * zoom, corner[i][1]);
		glVertex2f(corner[i][0], corner[i][1] + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(corner[i][0] + SHAPE_SIZE_RESIZE * zoom, corner[i][1]);
		glEnd();
	}
}
