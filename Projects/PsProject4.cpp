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
** Scan the work area to determine which cursor to display when using the "modify" tool.
** If the boolean "set" is true, the user has clicked, so we change mode in addition to possibly changing the cursor.
*/
PsController::Tool PsProjectController::ToolModifyScan(int x, int y, bool set)
{
	float fx;
	float fy;

	this->renderer.Convert(x, y, fx, fy);

	if (this->bPatternsIsSelected && !this->pattern->hide)
	{
		PsLayer* layer = this->pattern->aLayers[this->iLayerId];

		if (layer->InRotate(fx, fy, this->renderer.zoom, this->init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				this->prev_r = layer->rRotation.Yaw;
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}

		if (layer->MouseIsInside(fx, fy))
		{
			if (set)
			{
				this->prev_x = layer->vTranslation.X;
				this->prev_y = layer->vTranslation.Y;

				/*
				** Preparation of projection parameters
				*/
				PsVector eye = this->renderer.GetEyeLocation();
				PsVector direction(fx, fy, 0);
				direction -= eye;
				PsVector normal(0.f, 0.f, 1.f);

				/*
				** Intersection point of mouse click & plane at z = 0
				*/
				PsVector vZ0 = layer->vTranslation;
				vZ0.Z = 0.f;
				PsVector* p = LinePlaneIntersection(eye, direction, vZ0, normal);
				if (p)
				{
					this->prev_origin_z0 = *p;
					delete p;
				}

				/*
				** Intersection point of mouse click & PsLayer
				*/
				normal = RotateVertex(normal, PsRotator::FromDegree(layer->rRotation.Roll),
					PsRotator::FromDegree(layer->rRotation.Pitch), PsRotator::FromDegree(layer->rRotation.Yaw));
				p = LinePlaneIntersection(eye, direction, layer->vTranslation, normal);
				if (p)
				{
					this->prev_origin = *p;
					delete p;
				}
			}

			PsController::Instance().SetCursor(CURSOR_MOVE);
			return PsController::TOOL_MODIFY_MOVE;
		}

		PsController::Instance().SetCursor(CURSOR_DEFAULT);
		return PsController::TOOL_MODIFY;
	}

	if (this->image && !this->image->hide && (!this->matrix || !this->matrix->hide))
	{
		if (this->image->InResize(fx, fy, this->renderer.zoom, this->init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 + (int)((this->image->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				this->SelectImage(this->image);
				this->image->GetPosition(this->prev_x, this->prev_y);
				this->prev_h = this->image->h;
				this->prev_w = this->image->w;
			}

			return PsController::TOOL_MODIFY_SIZE;
		}
		else if (this->image->InRotate(fx, fy, this->renderer.zoom, this->init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((this->image->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				this->SelectImage(this->image);
				this->prev_r = this->image->GetAngle();
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}
	}

	if (this->matrix && !this->matrix->hide)
	{
		if (this->matrix->InResize(fx, fy, this->renderer.zoom, this->init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 + (int)((this->matrix->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				this->SelectMatrix(this->matrix);
				this->matrix->GetPosition(this->prev_x, this->prev_y);
				this->prev_h = this->matrix->h;
				this->prev_w = this->matrix->w;
			}

			return PsController::TOOL_MODIFY_SIZE;
		}
		else if (this->matrix->InTorsion(fx, fy, this->renderer.zoom, this->init_corner))
		{
			if (set) PsController::Instance().SetCursor(CURSOR_TORSIO2);
			else PsController::Instance().SetCursor(CURSOR_TORSIO1);

			if (set)
			{
				this->SelectMatrix(this->matrix);
				this->prev_i = this->matrix->i;
				this->prev_j = this->matrix->j;
			}

			return PsController::TOOL_MODIFY_TORSIO;
		}
		else if (this->matrix->InRotate(fx, fy, this->renderer.zoom, this->init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((this->matrix->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				this->SelectMatrix(this->matrix);
				this->prev_r = this->matrix->r;
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}
	}

	if (PsController::Instance().GetOption(PsController::OPTION_AUTOMATIC) || (!this->image && !this->matrix))
	{
		ImageList::reverse_iterator j;
		for (j = this->images.rbegin(); j != this->images.rend(); ++j)
			if (!(*j)->hide && (*j)->InContent(fx, fy))
			{
				PsController::Instance().SetCursor(CURSOR_MOVE);
				if (set)
				{
					this->SelectImage(*j);
					(*j)->GetPosition(this->prev_x, this->prev_y);
				}
				return PsController::TOOL_MODIFY_MOVE;
			}
		MatrixList::reverse_iterator i;
		for (i = this->matrices.rbegin(); i != this->matrices.rend(); ++i)
		{
			for (j = (*i)->images.rbegin(); !(*i)->hide && j != (*i)->images.rend(); ++j)
				if (!(*j)->hide && (*j)->InContent(fx, fy))
				{
					PsController::Instance().SetCursor(CURSOR_MOVE);
					if (set)
					{
						this->SelectImage(*j);
						(*j)->GetPosition(this->prev_x, this->prev_y);
					}
					return PsController::TOOL_MODIFY_MOVE;
				}
		}
		for (i = this->matrices.rbegin(); i != this->matrices.rend(); ++i)
			if (!(*i)->hide && (*i)->InContent(fx, fy))
			{
				PsController::Instance().SetCursor(CURSOR_MOVE);
				if (set)
				{
					this->SelectMatrix(*i);
					(*i)->GetPosition(this->prev_x, this->prev_y);
				}
				return PsController::TOOL_MODIFY_MOVE;
			}
	}
	else if (this->image && !this->image->hide && !this->matrix->hide)
	{
		PsController::Instance().SetCursor(CURSOR_MOVE);

		if (set)
		{
			this->SelectImage(this->image);
			this->image->GetPosition(this->prev_x, this->prev_y);
		}

		return PsController::TOOL_MODIFY_MOVE;
	}
	else if (this->matrix && !this->matrix->hide)
	{
		PsController::Instance().SetCursor(CURSOR_MOVE);

		if (set)
		{
			this->SelectMatrix(this->matrix);
			this->matrix->GetPosition(this->prev_x, this->prev_y);
		}

		return PsController::TOOL_MODIFY_MOVE;
	}

	PsController::Instance().SetCursor(CURSOR_DEFAULT);

	return PsController::TOOL_MODIFY;
}

/*
** Movement of the "drag" tool.
*/
void PsProjectController::ToolScrollDrag(int x, int y, int prev_x, int prev_y)
{
	this->renderer.SetScroll(this->prev_scrollx + (prev_x - x) * this->renderer.zoom, this->prev_scrolly + (prev_y - y) * this->renderer.zoom);
	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogOverview();
}

/*
** Initialization of the "drag" tool.
*/
PsController::Tool PsProjectController::ToolScrollStart()
{
	this->renderer.GetScroll(this->prev_scrollx, this->prev_scrolly);
	return PsController::TOOL_SCROLL_DRAG;
}

/*
** Returns the width of the document in pixels.
*/
int PsProjectController::GetWidth()
{
	return this->renderer.doc_x;
}

/*
** Returns the height of the document in pixels.
*/
int PsProjectController::GetHeight()
{
	return this->renderer.doc_y;
}

/*
** Returns the number of pixels per inch in the document.
*/
int PsProjectController::GetDpi()
{
	return this->renderer.dpi;
}
