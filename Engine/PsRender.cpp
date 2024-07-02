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

/*
 ** Dessine une boite de sélection autour d'une matrice(uniquement hardware).
 */
void		PsRender::DrawBox(const PsMatrix& matrix)
{
	float	corner[4][2];
	float	r;
	float	g;
	float	b;
	int		i;

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
 ** Dessine les contours du document(uniquement hardware).
 */
void	PsRender::DrawDocument()
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
	glVertex2f(0, y1);
	glVertex2f(0, y2);
	glVertex2f(x1, y2);
	glVertex2f(x1, y1);

	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f((float)doc_x, y2);
	glVertex2f((float)doc_x, y1);

	glVertex2f(0, 0);
	glVertex2f((float)doc_x, 0);
	glVertex2f((float)doc_x, y2);
	glVertex2f(0, y2);

	glVertex2f(0, y1);
	glVertex2f((float)doc_x, y1);
	glVertex2f((float)doc_x, (float)doc_y);
	glVertex2f(0, (float)doc_y);
	glEnd();

	glDisable(GL_BLEND);
	glColor3f(0.25f, 0.25f, 0.25f);

	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0f, 0.0f);
	glVertex2f((float)doc_x, 0.0f);
	glVertex2f((float)doc_x, (float)doc_y);
	glVertex2f(0.0f, (float)doc_y);
	glEnd();
}

/*
 ** Dessine un patron, soit en mode normal("derrière" la scène, et en positif), soit en
 ** mode masque("devant" la scène, en négatif).
 */
void	PsRender::DrawPattern(PsPattern& pattern/*, bool mask*/)
{

	if (engine == ENGINE_HARDWARE)
	{
		glColor4f(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		// arrière plan
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, pattern.texture.GetID());
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, 0);
		glTexCoord2f(1, 1);
		glVertex2f((float)doc_x, 0);
		glTexCoord2f(1, 0);
		glVertex2f((float)doc_x, (float)doc_y);
		glTexCoord2f(0, 0);
		glVertex2f(0, (float)doc_y);
		glEnd();

		// ombre  
		glColor4f(0, 0, 0, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, pattern.y_map_texture_id);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, 0);
		glTexCoord2f(1, 1);
		glVertex2f((float)doc_x, 0);
		glTexCoord2f(1, 0);
		glVertex2f((float)doc_x, (float)doc_y);
		glTexCoord2f(0, 0);
		glVertex2f(0, (float)doc_y);
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
 ** Dessine une image à la position absolue x, y. Le booléen "precalc" indique si il faut ou non
 ** calculer les coordonnées des coins de l'image(à ajouter à x et y), vu qu'ils restent les mêmes
 ** pour une image, il est inutile de les recalculer à chaque fois qu'elle est affichée, seuls x et y
 ** changent.
 */
void	PsRender::DrawShape(PsImage& image, float x, float y, bool bFirst, bool bLast)
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

	if (engine == ENGINE_HARDWARE)
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

bool PsRender::IsInside(int x, int y) const
{
	return x >= x1 && x <= x2 && y >= y2 && y <= y1;
}

bool PsRender::IsInside(const PsImage& image, int x, int y) const
{
	if (IsInside(x + FloatToInt(image.corner[0][0]), y + FloatToInt(image.corner[0][1])))
		return true;
	else if (IsInside(x + FloatToInt(image.corner[1][0]), y + FloatToInt(image.corner[1][1])))
		return true;
	else if (IsInside(x + FloatToInt(image.corner[2][0]), y + FloatToInt(image.corner[2][1])))
		return true;
	else if (IsInside(x + FloatToInt(image.corner[3][0]), y + FloatToInt(image.corner[3][1])))
		return true;
	return false;
}

/*
 ** Permet d'obtenir les dimensions orthonormée d'une matrice
 */
void  PsRender::GetMatrixWindow(PsMatrix& matrix, double& left, double& right, double& bottom, double& top)
{
	float corner[4][4];
	matrix.ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	matrix.ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	matrix.ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	matrix.ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);
	left = corner[0][0];
	for (int i = 0; i < 4; ++i)
		if (corner[i][0] < left)
			left = corner[i][0];
	right = corner[0][0];
	for (int i = 0; i < 4; ++i)
		if (corner[i][0] > right)
			right = corner[i][0];
	bottom = corner[0][1];
	for (int i = 0; i < 4; ++i)
		if (corner[i][1] > bottom)
			bottom = corner[i][1];
	top = corner[0][1];
	for (int i = 0; i < 4; ++i)
		if (corner[i][1] < top)
			top = corner[i][1];
}

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

void PsRender::DrawStandardGizmos(PsProject& project)
{
	DrawMatricesGizmos(project);

	if (project.image && !project.image->hide && !project.matrix)
	{
		glColor4f(1.f, 1.f, 1.f, 0.8f);
		DrawShape(*project.image);
	}

	if (project.image && !project.image->hide &&
		(!project.matrix || !project.matrix->hide))
	{
		DrawBox(*project.image);
	}

	if (project.matrix && !project.matrix->hide)
		DrawBoxHandle(*project.matrix);
}

void PsRender::DrawGizmos(PsProject& project)
{
	if (project.bPatternsIsSelected && project.pattern && !project.pattern->hide)
	{
		DrawPatternGizmo(*project.pattern, project.iLayerId);
	}
	else
	{
		DrawStandardGizmos(project);
	}
}

void PsRender::MonoLayerRendering(PsProject& project, int x, int y)
{
	PrepareSurface(project, x, y);

	if (engine == ENGINE_HARDWARE)
		DrawBack(project, (float)x, (float)y);

	DrawMatrices(project);

	if (project.pattern && !project.pattern->hide)
		DrawPattern(*project.pattern/*, true*/);
}

void PsRender::DrawLayerTexture(GLuint layer)
{
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, layer);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, doc_y);
	glTexCoord2f(1, 0);
	glVertex2f(doc_x, doc_y);
	glTexCoord2f(1, 1);
	glVertex2f(doc_x, 0);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}



void PsRender::UpdateLayerTexture(PsProject& project, PsLayer* layer, GLuint iDocTexture)
{
	PsPattern* pattern = project.pattern;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, iLayerTextureSize, iLayerTextureSize);
	glOrtho(0, doc_x, doc_y, 0, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//-- affichage du plan
	float fDup = 9.f;
	float fScale = layer->fScale;
	layer->fScale *= fDup;
	layer->UpdateProjection(*this);
	layer->fScale = fScale;
	PsVector* vCorners = layer->vProjected;
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, iDocTexture);
	glBegin(GL_QUADS);
	glTexCoord4f(0.f * layer->fW[0], fDup * layer->fW[0], 0.f, layer->fW[0]);
	glVertex2f(vCorners[0].X, vCorners[0].Y);
	glTexCoord4f(fDup * layer->fW[1], fDup * layer->fW[1], 0.f, layer->fW[1]);
	glVertex2f(vCorners[1].X, vCorners[1].Y);
	glTexCoord4f(fDup * layer->fW[2], 0.f, 0.f, layer->fW[2]);
	glVertex2f(vCorners[2].X, vCorners[2].Y);
	glTexCoord4f(0.f, 0.f, 0.f, layer->fW[3]);
	glVertex2f(vCorners[3].X, vCorners[3].Y);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	layer->UpdateProjection(*this);
	//--

	//-- affichage du masque
	if (engine == ENGINE_HARDWARE)
	{
		glColor4f(1.f, 1.f, 1.f, 1.f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, layer->iMaskTextureId);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, doc_y);
		glTexCoord2f(1, 1);
		glVertex2f(doc_x, doc_y);
		glTexCoord2f(1, 0);
		glVertex2f(doc_x, 0);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	//--

	hardwareRenderer.CopyToHardBuffer(layer->iFinalTextureId, iLayerTextureSize, iLayerTextureSize, layer->iFinalTextureId == NULL);
	PrepareSurface(project, size_x, size_y);
}

/*
 ** Récupération du motif répété sur tout le document dans une
 ** texture de taille 'fTextureSize'.
 */
GLuint PsRender::CreateDocumentTexture(PsProject& project)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, iLayerTextureSize, iLayerTextureSize);
	glOrtho(0, doc_x, doc_y, 0, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawMatrices(project);
	GLuint iTextureBloc;
	hardwareRenderer.CopyToHardBuffer(iTextureBloc, iLayerTextureSize, iLayerTextureSize);
	PrepareSurface(project, size_x, size_y);
	return iTextureBloc;
}

/*
 ** Renvoie la position de l'oeil utilisée en mode MultiLayerRendering (voir PsPattern)
 ** pour projeter les différentes couches (voir PsLayer).
 */
PsVector PsRender::GetEyeLocation()
{
	return PsVector(doc_x / 2.f, doc_y / 2.f, doc_y * 2.415f);
}

void PsRender::MultiLayerRendering(PsProject& project, int x, int y)
{
	// pré-conditions
	if (!project.pattern) return;

	//-- récupération des paramètres
	PsPattern* pattern = project.pattern;
	int iCount = pattern->GetChannelsCount();
	//--

	//iCount = 1; // FIXME

	this->PrepareSurface(project, x, y);

	if (iCount > 0)
	{
		//-- mode HARDWARE
		if (engine == ENGINE_HARDWARE)
		{
#ifdef _MACOSX
			active->SetPixelBufferContext();
#endif

			GLuint iDocTexture = this->CreateDocumentTexture(project);
			for (int i = 0; i < iCount; ++i)
			{
				this->UpdateLayerTexture(project, pattern->aLayers[i], iDocTexture);
			}

#ifdef _MACOSX
			active->RestorePreviousContext();
#endif

			this->DrawBack(project, (float)x, (float)y);
			for (int i = 0; i < iCount; ++i)
			{
				this->DrawLayerTexture(pattern->aLayers[i]->iFinalTextureId);
			}

			glDeleteTextures(1, &iDocTexture);

		}
		//-- sinon mode SOFTWARE
		else
		{
			for (int i = 0; i < iCount; ++i)
			{
				DrawLayerSoftwareFile(i);
			}
		}
		//--
	}

	this->DrawPattern(*project.pattern);
}

/**
 * Déclanche un rendu, hardware ou software, selon le mode choisi.
*/
void PsRender::Render(PsProject& project, int x, int y)
{

	if (engine == ENGINE_HARDWARE)
	{
		glPushMatrix();
	}

	if (!project.pattern
		|| project.pattern->hide)
	{
		this->MonoLayerRendering(project, x, y);
	}
	else
	{
		this->MultiLayerRendering(project, x, y);
	}

	this->DrawImages(project);

	if (engine == ENGINE_HARDWARE)
	{
		if (PsController::Instance().GetOption(PsController::OPTION_DOCUMENT_SHOW))
		{
			this->DrawDocument();
		}

		if (PsController::Instance().GetOption(PsController::OPTION_BOX_SHOW))
		{
			this->DrawGizmos(project);
		}
		glPopMatrix();
	}
}

/*
 ** Change la taille du document.
 */
void	PsRender::SetDocSize(int x, int y)
{
	doc_x = x;
	doc_y = y;

	if (PsController::Instance().project && PsController::Instance().project->pattern)
		PsController::Instance().project->pattern->UpdateScale(doc_x, doc_y);
}

/*
 ** Change le mode de rendu(software, hardware).
 */
void	PsRender::SetEngine(Engine engine)
{
	this->engine = engine;
}

/*
 ** Change les valeurs de scrolling.
 */
void	PsRender::SetScroll(float x, float y)
{
	scroll_x = x;
	scroll_y = y;

	Recalc();
}

/*
 ** Change la taille de la fenêtre.
 */
void	PsRender::SetSize(int x, int y)
{
	size_x = x;
	size_y = y;

	int iMax = size_x;
	if (size_y > iMax) iMax = size_y;

	fZoomMax = 1.5f / ((float)y / (float)doc_y);
	fZoomMin = 50.f / (float)iMax;

	Recalc();
}

/*
 ** Définit manuellement une zone de document(utilisé pour le rendu software).
 */
void PsRender::SetZone(float x, float y)
{
	x1 = 0;
	x2 = x;
	y1 = y;
	y2 = 0;
}

/*
 ** Change le zoom.
 */
void PsRender::SetZoom(float zoom)
{
	if (zoom < fZoomMin)
		zoom = fZoomMin;
	else if (zoom > fZoomMax)
		zoom = fZoomMax;

	this->zoom = zoom;

	Recalc();
}
