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
