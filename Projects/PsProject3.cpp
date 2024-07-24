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
** Déclanche l'affichage du projet dans le contexte actif, en utilisant un rendu
** hardware(donc OpenGL).
*/
void	PsProject::RenderToScreen()
{
	int	x;
	int	y;

	if (center)
	{
		renderer.CenterView();
		center = false;
	}

	renderer.GetSize(x, y);
	renderer.SetEngine(PsRender::ENGINE_HARDWARE);
	renderer.Render(*this, x, y);

#ifdef _WINDOWS
	int m_glErrorCode = glGetError();
	if (m_glErrorCode != GL_NO_ERROR)
	{
		const GLubyte* estring;
		CString mexstr;
		estring = gluErrorString(m_glErrorCode);
		mexstr.Format("RenderToScreen:\n\tAn OpenGL error occurred: %s\n", estring);
		AfxMessageBox(mexstr, MB_OK | MB_ICONEXCLAMATION);
		TRACE0(mexstr);
	}
#endif /* _WINDOWS */
}

/*
** Déclanche le rendu dans un fichier
*/
bool	PsProject::RenderToFile(const char* filename, int x, int y)
{
	PsController::Instance().SetProgress(-1);

	renderer.SetZone((float)GetWidth(), (float)GetHeight());

	InitSoftwareFile(x, y);

	PsController::Instance().SetProgress(5);

	renderer.SetEngine(PsRender::ENGINE_SOFTWARE);
	renderer.Render(*this, x, y);
	renderer.Recalc();

	PsController::Instance().SetProgress(90);

	flushSoftwareFile(filename, bHideColor);

	PsController::Instance().SetProgress(-2);

	return true;
}

/*
** Sélectionne une image dans le projet(et la matrice qui la contient).
*/
void	PsProject::SelectImage(PsImage* image)
{
	this->matrix = image ? image->GetParent() : 0;
	this->image = image;
	this->shape = image;

	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogProject();

	if (dlgPropreties)
	{
		//dlgPropreties->FocusImageInformation();
		dlgPropreties->UpdateInformation(this);
	}
}

/*
** Sélectionne une matrice dans le projet.
*/
void	PsProject::SelectMatrix(PsMatrix* matrix)
{
	this->matrix = matrix;
	this->image = 0;
	this->shape = matrix;

	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogProject();

	if (dlgPropreties)
	{
		//dlgPropreties->FocusMatrixInformation();
		dlgPropreties->UpdateInformation(this);
	}
}

/*
** Déplacement de l'outil "loupe"(magnify).
*/
void		PsProject::ToolMagnifyDrag(int y, int old_x, int old_y)
{
	float	zoom = prev_zoom + (y - old_y) * ZOOM_COEF;

	if (y > old_y)
		PsController::Instance().SetCursor(CURSOR_MAGNIFY2);
	else
		PsController::Instance().SetCursor(CURSOR_MAGNIFY3);

	/* static float fx, fy, ox, oy;
	if (prev_zoom == renderer.zoom)
	{
		renderer.Convert(old_x, old_y, fx, fy);
		ox = renderer.scroll_x;
		oy = renderer.scroll_y;
	} */

	renderer.SetZoom(zoom);

	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogOverview();
}

/*
** Initialisation de l'outil "loupe".
*/
PsController::Tool	PsProject::ToolMagnifyStart()
{
	prev_zoom = renderer.zoom;

	return PsController::TOOL_MAGNIFY_ZOOM;
}

/*
** Déplacement de l'outil "Modify"(qui a besoin de savoir dans quel mode il est, pour
** connaitre l'operation à effectuer parmis déplacement, rotation, redimentionnement...)
*/
void		PsProject::ToolModifyMove(int x, int y, int old_x, int old_y, PsController::Tool tool)
{
	float	fx;
	float	fy;
	float	sx;
	float	sy;
	float	tx;
	float	ty;
	bool	constrain = PsController::Instance().GetOption(PsController::OPTION_CONSTRAIN);
	bool	reflect = PsController::Instance().GetOption(PsController::OPTION_REFLECT);


	renderer.Convert(x, y, fx, fy);
	renderer.Convert(old_x, old_y, sx, sy);

	if (bPatternsIsSelected && !pattern->hide)
	{
		PsLayer* layer = pattern->aLayers[iLayerId];

		switch (tool)
		{
		case PsController::TOOL_MODIFY_ROTATE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogPatternRotate(*this));

			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));
			layer->rRotation.Yaw = prev_r + PsRotator::ToDegree(-(layer->ToAngle(fx, fy) - layer->ToAngle(sx, sy)));
			break;
		}

		case PsController::TOOL_MODIFY_MOVE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogPatternTranslate(*this));

			PsVector vTranslation(fx - sx, fy - sy, 0.f);

			PsLayer* layer = pattern->aLayers[iLayerId];

			PsVector vEye = renderer.GetEyeLocation();

			float fDistance = (prev_origin - vEye).Size();
			float fDistanceZ0 = (prev_origin_z0 - vEye).Size();

			vTranslation = vTranslation * (fDistance / fDistanceZ0);

			layer->vTranslation.X = prev_x + vTranslation.X;
			layer->vTranslation.Y = prev_y + vTranslation.Y;

			break;
		}

		case PsController::TOOL_MODIFY_SIZE:
		{
			// FIXME : LOG !!

			tx = (fx - sx) * cos(-shape->GetAngle()) - (fy - sy) * sin(-shape->GetAngle());
			ty = (fx - sx) * sin(-shape->GetAngle()) + (fy - sy) * cos(-shape->GetAngle());

			/*
				if (init_corner == 0)
				layer->SetSize(prev_w - tx, prev_h - ty, -SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 1)
				layer->SetSize(prev_w + tx, prev_h - ty, SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 2)
				layer->SetSize(prev_w - tx, prev_h + ty, -SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else
				layer->SetSize(prev_w + tx, prev_h + ty, SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			*/
			break;
		}

		}
	}
	else
	{

		if (!shape || shape->hide)
			return;

		switch (tool)
		{
		case PsController::TOOL_MODIFY_MOVE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogMove(*this, shape, prev_x, prev_y));

			shape->SetPosition(prev_x + fx - sx, prev_y + fy - sy, constrain);
			break;
		}

		case PsController::TOOL_MODIFY_ROTATE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogRotate(*this, shape, prev_r, reflect));

			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((shape->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			shape->SetAngle(prev_r + shape->ToAngle(fx, fy) - shape->ToAngle(sx, sy), constrain, reflect);
			break;
		}

		case PsController::TOOL_MODIFY_SIZE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogResize(*this, shape, prev_x, prev_y, prev_w, prev_h, reflect));

			tx = (fx - sx) * cos(-shape->GetAngle()) - (fy - sy) * sin(-shape->GetAngle());
			ty = (fx - sx) * sin(-shape->GetAngle()) + (fy - sy) * cos(-shape->GetAngle());

			if (init_corner == 0)
				shape->SetSize(prev_w - tx, prev_h - ty, -SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 1)
				shape->SetSize(prev_w + tx, prev_h - ty, SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 2)
				shape->SetSize(prev_w - tx, prev_h + ty, -SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else
				shape->SetSize(prev_w + tx, prev_h + ty, SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			break;
		}

		case PsController::TOOL_MODIFY_TORSIO:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogTorsio(*this, shape, prev_i, prev_j));

			tx = fx - sx;
			ty = fy - sy;

			if (init_corner == 0)
				shape->SetTorsion(prev_i - tx * cos(shape->GetAngle()) - ty * sin(shape->GetAngle()), prev_j, constrain);
			else if (init_corner == 1)
				shape->SetTorsion(prev_i, prev_j - tx * sin(shape->GetAngle()) + ty * cos(shape->GetAngle()), constrain);
			else if (init_corner == 2)
				shape->SetTorsion(prev_i + tx * cos(shape->GetAngle()) + ty * sin(shape->GetAngle()), prev_j, constrain);
			else
				shape->SetTorsion(prev_i, prev_j + tx * sin(shape->GetAngle()) - ty * cos(shape->GetAngle()), constrain);
			break;
		}
		}
	}

	PsController::Instance().UpdateWindow();
	if (dlgPropreties)
		dlgPropreties->UpdateInformation(this);
}

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