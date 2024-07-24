// PsProject.cpp

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

#include <stdio.h>
#include "PsProject.h"
#include "PsController.h"
#include "PsAction.h"
#include "PsMatrix.h"
#include "PsPattern.h"
#include "PsSoftRender.h"
#include "PsWinPropertiesCx.h"

/*
** Scan de la zone de travail pour savoir quel curseur à afficher lors de l'utilisaton de l'outil
** "modify". Si le booleen "set" est à vrai, l'utilisateur a cliqué, donc on change de mode en plus
** de changer éventuellement de curseur.
*/
PsController::Tool PsProject::ToolModifyScan(int x, int y, bool set)
{
	float									fx;
	float									fy;

	renderer.Convert(x, y, fx, fy);

	if (bPatternsIsSelected && !pattern->hide)
	{
		PsLayer* layer = pattern->aLayers[iLayerId];

		/*
		if (layer->InResize(fx, fy, renderer.zoom, init_corner))
		 {
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 +(int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				prev_x = layer->vTranslation.X;
				prev_y = layer->vTranslation.Y;
				prev_h = layer->fScale * layer->fGizmoSize;
				prev_w = layer->fScale * layer->fGizmoSize;
			}

			return PsController::TOOL_MODIFY_SIZE;
		 }
		 */

		if (layer->InRotate(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				prev_r = layer->rRotation.Yaw;
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}

		if (layer->MouseIsInside(fx, fy))
		{
			if (set)
			{
				prev_x = layer->vTranslation.X;
				prev_y = layer->vTranslation.Y;

				/*
				** Préparation des paramètres de projection
				 */
				PsVector oeil = renderer.GetEyeLocation();
				PsVector direction(fx, fy, 0);
				direction -= oeil;
				PsVector normal(0.f, 0.f, 1.f);

				/*
				** Point d'intersection click souris & plan en z = 0
				 */
				PsVector vZ0 = layer->vTranslation;
				vZ0.Z = 0.f;
				PsVector* p = LinePlaneIntersection(oeil, direction, vZ0, normal);
				if (p)
				{
					prev_origin_z0 = *p;
					delete p;
				}

				/*
				** Point d'intersection click souris & PsLayer
				*/
				normal = RotateVertex(normal, PsRotator::FromDegree(layer->rRotation.Roll),
					PsRotator::FromDegree(layer->rRotation.Pitch), PsRotator::FromDegree(layer->rRotation.Yaw));
				p = LinePlaneIntersection(oeil, direction, layer->vTranslation, normal);
				if (p)
				{
					prev_origin = *p;
					delete p;
				}


			}

			PsController::Instance().SetCursor(CURSOR_MOVE);
			return PsController::TOOL_MODIFY_MOVE;
		}

		PsController::Instance().SetCursor(CURSOR_DEFAULT);
		return PsController::TOOL_MODIFY;
	}

	if (image && !image->hide && (!matrix || !matrix->hide))
	{
		if (image->InResize(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 + (int)((image->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				SelectImage(image);
				image->GetPosition(prev_x, prev_y);
				prev_h = image->h;
				prev_w = image->w;
			}

			return PsController::TOOL_MODIFY_SIZE;
		}
		else if (image->InRotate(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((image->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				SelectImage(image);
				prev_r = image->GetAngle();
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}
	}

	if (matrix && !matrix->hide)
	{
		if (matrix->InResize(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 + (int)((matrix->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				SelectMatrix(matrix);
				matrix->GetPosition(prev_x, prev_y);
				prev_h = matrix->h;
				prev_w = matrix->w;
			}

			return PsController::TOOL_MODIFY_SIZE;
		}
		else if (matrix->InTorsion(fx, fy, renderer.zoom, init_corner))
		{
			if (set) PsController::Instance().SetCursor(CURSOR_TORSIO2);
			else PsController::Instance().SetCursor(CURSOR_TORSIO1);

			if (set)
			{
				SelectMatrix(matrix);
				prev_i = matrix->i;
				prev_j = matrix->j;
			}

			return PsController::TOOL_MODIFY_TORSIO;
		}
		else if (matrix->InRotate(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((matrix->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				SelectMatrix(matrix);
				prev_r = matrix->r;
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}
	}

	if (PsController::Instance().GetOption(PsController::OPTION_AUTOMATIC) || (!image && !matrix))
	{
		ImageList::reverse_iterator j;
		for (j = images.rbegin(); j != images.rend(); ++j)
			if (!(*j)->hide && (*j)->InContent(fx, fy))
			{
				PsController::Instance().SetCursor(CURSOR_MOVE);
				if (set)
				{
					SelectImage(*j);
					(*j)->GetPosition(prev_x, prev_y);
				}
				return PsController::TOOL_MODIFY_MOVE;
			}
		MatrixList::reverse_iterator i;
		for (i = matrices.rbegin(); i != matrices.rend(); ++i)
		{
			for (j = (*i)->images.rbegin(); !(*i)->hide && j != (*i)->images.rend(); ++j)
				if (!(*j)->hide && (*j)->InContent(fx, fy))
				{
					PsController::Instance().SetCursor(CURSOR_MOVE);
					if (set)
					{
						SelectImage(*j);
						(*j)->GetPosition(prev_x, prev_y);
					}
					return PsController::TOOL_MODIFY_MOVE;
				}
		}
		for (i = matrices.rbegin(); i != matrices.rend(); ++i)
			if (!(*i)->hide && (*i)->InContent(fx, fy))
			{
				PsController::Instance().SetCursor(CURSOR_MOVE);
				if (set)
				{
					SelectMatrix(*i);
					(*i)->GetPosition(prev_x, prev_y);
				}
				return PsController::TOOL_MODIFY_MOVE;
			}
	}
	else if (image && !image->hide && !matrix->hide)
	{
		PsController::Instance().SetCursor(CURSOR_MOVE);

		if (set)
		{
			SelectImage(image);
			image->GetPosition(prev_x, prev_y);
		}

		return PsController::TOOL_MODIFY_MOVE;
	}
	else if (matrix && !matrix->hide)
	{
		PsController::Instance().SetCursor(CURSOR_MOVE);

		if (set)
		{
			SelectMatrix(matrix);
			matrix->GetPosition(prev_x, prev_y);
		}

		return PsController::TOOL_MODIFY_MOVE;
	}

	PsController::Instance().SetCursor(CURSOR_DEFAULT);

	return PsController::TOOL_MODIFY;
}

/*
** Déplacement de l'outil "drag".
*/
void PsProject::ToolScrollDrag(int x, int y, int prev_x, int prev_y)
{
	renderer.SetScroll(prev_scrollx + (prev_x - x) * renderer.zoom, prev_scrolly + (prev_y - y) * renderer.zoom);
	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogOverview();
}

/*
** Initialisation de l'outil "drag".
*/
PsController::Tool PsProject::ToolScrollStart()
{
	renderer.GetScroll(prev_scrollx, prev_scrolly);
	return PsController::TOOL_SCROLL_DRAG;
}

/*
** Retourne la largeur du document en pixels.
*/
int PsProject::GetWidth()
{
	return renderer.doc_x;
}

/*
** Retourne la hauteur du document en pixels.
*/
int PsProject::GetHeight()
{
	return renderer.doc_y;
}

/*
** Retourne le nombre de pixels par pouce dans le document.
*/
int PsProject::GetDpi()
{
	return renderer.dpi;
}