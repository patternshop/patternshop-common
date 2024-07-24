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
 ** Dessine le contenu d'une matrice(la matrice elle-même n'est pas représentable, sauf éventuellement
									 ** un cadre coloré pour indiquer la matrice centrale), et répete cet affichage sur tout l'écran afin
 ** de dessiner un réseau complet. Au final, cette fonction ne fait qu'appeler DrawShape pour chaque image,
 ** autant de fois que nécessaire.
 */
void  PsRender::DrawShape(PsMatrix& matrix)
{
	if (matrix.hide)
		return;

	//-- taille du coté le plus petit de la matrice
	double minSize = matrix.w;
	if (minSize > matrix.h)
		minSize = matrix.h;
	//--

	//-- taille de la diagonale de la zone de rendu
	double l, r, b, t;
	GetMatrixWindow(matrix, l, r, b, t);
	if (x2 > r) r = x2;
	if (y1 > b) b = y1;
	if (x1 < l) l = x1;
	if (y2 < t) t = y2;
	double dw = (r - l);
	double dh = (b - t);
	double powDiag = sqrt(pow(dw, 2) + pow(dh, 2));
	//--

	//-- on double le min(FIXME, c'est quick and dirty)
	double quickDup = round((powDiag / minSize) * 2);
	double quickDupMax = 300;
	if (quickDup > quickDupMax)
	{
		//warning ?
		quickDup = quickDupMax;
	}
	//--

	int iMaximum = DoubleToInt(quickDup);

	float cos_r = cos(matrix.r);
	float sin_r = sin(matrix.r);
	float i2 = matrix.i;
	float j2 = matrix.j;
	float mh = matrix.h;
	float mw = matrix.w;

	glColor4f(1.f, 1.f, 1.f, 1.f);
	ImageList::const_iterator image;
	for (image = matrix.images.begin(); image != matrix.images.end(); ++image)
	{
		bool bFirst = true;
		for (int y = -iMaximum; y < iMaximum; ++y)
		{
			float yi2 = y * i2;
			float ymh = y * mh;
			for (int x = -iMaximum; x <= iMaximum; ++x)
			{
				float xj2 = x * j2;
				float xmw = x * mw;
				float tx = (xmw + yi2) * cos_r - (ymh + xj2) * sin_r;
				float ty = (xmw + yi2) * sin_r + (ymh + xj2) * cos_r;
				DrawShape(**image, tx, ty, bFirst, false);
				if (bFirst) bFirst = false;
			}
		}
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}

/*
 ** Retourne le buffer dans lequel le rendu software a été fait (pour l'instant, c'est une capture
 ** du rendu hardware, ce qui en plus d'être une mauvaise solution d'un point de vue qualité interdit
 ** de capturer une zone plus grande que l'écran).
 */
uint8* PsRender::GetBuffer(int x, int y) const
{
	uint8* buffer = new uint8[x * y * 4];
	uint8	pixel;
	int				i;
	int				j;

	glFlush();
	glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	for (i = 0, j = x * y * 4; i < j; i += 4)
	{
		pixel = buffer[i + 0];
		buffer[i + 0] = buffer[i + 2];
		buffer[i + 2] = pixel;
	}

	return buffer;
}

/*
 ** Récupere la taille du document.
 */
void	PsRender::GetDocSize(int& x, int& y) const
{
	x = doc_x;
	y = doc_y;
}

/*
 ** Récupere les valeurs des scrolling actuelles.
 */
void	PsRender::GetScroll(float& x, float& y) const
{
	x = scroll_x;
	y = scroll_y;
}

/*
 ** Récupere la taille de la zone de vue.
 */
void	PsRender::GetSize(int& x, int& y) const
{
	x = size_x;
	y = size_y;
}

/*
 ** Recalcule d'un point de vue du document les coordonnées des bords de la fenêtre, en fonction
 ** du scrolling, du zoom, et de la taille de la fenêtre.
 */
void		PsRender::Recalc()
{
	float	zx = size_x * zoom / 2;
	float	zy = size_y * zoom / 2;

	x1 = scroll_x - zx;
	x2 = scroll_x + zx;
	y1 = scroll_y + zy;
	y2 = scroll_y - zy;
}

void PsRender::PrepareSurface(PsProject& project, int x, int y)
{
	float fMaxSize = doc_x > doc_y ? doc_x : doc_y;

	float fMaxWidth, hMaxHeight, fD1;
	fMaxWidth = fMaxSize;
	hMaxHeight = fMaxSize;
	fD1 = hMaxHeight / 4.f;

	if (engine == ENGINE_HARDWARE)
	{

		glViewport(0, 0, x, y);
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(x1, x2, y1, y2, -10000, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPopMatrix();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void PsRender::DrawMatrices(PsProject& project)
{
	MatrixList::const_iterator i;
	int n;

	for (n = 0, i = project.matrices.begin(); i != project.matrices.end(); ++i)
	{
		if (engine == ENGINE_SOFTWARE)
			PsController::Instance().SetProgress((int)(20 + 60 * n++ / project.matrices.size()));
		DrawShape(**i);
	}
}

void PsRender::DrawImages(PsProject& project)
{
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	ImageList::const_iterator j;
	for (j = project.images.begin(); j != project.images.end(); ++j)
		DrawShape(**j);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void PsRender::DrawMatricesGizmos(PsProject& project)
{
	MatrixList::const_iterator i;

	for (i = project.matrices.begin(); i != project.matrices.end(); ++i)
	{
		PsMatrix& matrix = **i;
		if (!matrix.hide)
		{
			DrawBox(matrix);
			glColor4f(1.f, 1.f, 1.f, 0.8f);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			ImageList::const_iterator	image;
			for (image = matrix.images.begin(); image != matrix.images.end(); ++image)
				DrawShape(**image);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
}

void PsRender::DrawPatternGizmo(PsPattern& pattern, int iIndex)
{
	PsLayer* layer = pattern.aLayers[iIndex];

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//-- cadrillage du calque
	layer->UpdateGizmoProjection(*this);
	PsVector* vCorners = layer->vProjected;
	glColor4f(0.0f, 0.0f, 0.2f, 0.5f);
	float p[4];
	float v[4];
	float div_x = 8, div_y = 8;
	v[0] = (vCorners[1].X - vCorners[0].X) / (float)div_x;
	v[1] = (vCorners[1].Y - vCorners[0].Y) / (float)div_x;
	v[2] = (vCorners[2].X - vCorners[3].X) / (float)div_x;
	v[3] = (vCorners[2].Y - vCorners[3].Y) / (float)div_x;
	for (int i = 0; i <= div_x; ++i)
	{
		p[0] = vCorners[0].X + v[0] * i;
		p[1] = vCorners[0].Y + v[1] * i;
		p[2] = vCorners[3].X + v[2] * i;
		p[3] = vCorners[3].Y + v[3] * i;
		glBegin(GL_LINE_LOOP);
		glVertex2f(p[0], p[1]);
		glVertex2f(p[2], p[3]);
		glEnd();
	}
	v[0] = (vCorners[3].X - vCorners[0].X) / (float)div_y;
	v[1] = (vCorners[3].Y - vCorners[0].Y) / (float)div_y;
	v[2] = (vCorners[2].X - vCorners[1].X) / (float)div_y;
	v[3] = (vCorners[2].Y - vCorners[1].Y) / (float)div_y;
	for (int i = 0; i <= div_y; ++i)
	{
		p[0] = vCorners[0].X + v[0] * i;
		p[1] = vCorners[0].Y + v[1] * i;
		p[2] = vCorners[1].X + v[2] * i;
		p[3] = vCorners[1].Y + v[3] * i;
		glBegin(GL_LINE_LOOP);
		glVertex2f(p[0], p[1]);
		glVertex2f(p[2], p[3]);
		glEnd();
	}
	//--


	//-- fond du gizmo
	layer->UpdateProjection(*this);
	vCorners = layer->vProjectedGizmo;
	glColor4f(0.0f, 0.8f, 0.0f, 0.2f);
	glBegin(GL_QUADS);
	glVertex2f(vCorners[0].X, vCorners[0].Y);
	glVertex2f(vCorners[1].X, vCorners[1].Y);
	glVertex2f(vCorners[2].X, vCorners[2].Y);
	glVertex2f(vCorners[3].X, vCorners[3].Y);
	glEnd();
	glColor4f(0.0f, 0.8f, 0.0f, 1.f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(vCorners[0].X, vCorners[0].Y);
	glVertex2f(vCorners[1].X, vCorners[1].Y);
	glVertex2f(vCorners[2].X, vCorners[2].Y);
	glVertex2f(vCorners[3].X, vCorners[3].Y);
	glEnd();
	//--

	//-- poignées du gizmo
	for (int i = 0; i < 4; ++i)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(vCorners[i].X - SHAPE_SIZE_RESIZE * zoom, vCorners[i].Y - SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(vCorners[i].X - SHAPE_SIZE_RESIZE * zoom, vCorners[i].Y + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(vCorners[i].X + SHAPE_SIZE_RESIZE * zoom, vCorners[i].Y + SHAPE_SIZE_RESIZE * zoom);
		glVertex2f(vCorners[i].X + SHAPE_SIZE_RESIZE * zoom, vCorners[i].Y - SHAPE_SIZE_RESIZE * zoom);
		glEnd();
	}
	//--

	glDisable(GL_BLEND);
}
