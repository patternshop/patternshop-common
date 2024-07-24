// PsImage.cpp

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

#include "PsImage.h"
#include "PsMatrix.h"
#include "PsMaths.h"

/*
** An image is always contained in a matrix, so it uses the corresponding constructor of PsShape.
*/
PsImage::PsImage(PsMatrix* parent) :
	PsShape(*parent)
{
}

PsImage::~PsImage()
{
}

/*
** Returns the matrix in which the image is contained.
*/
PsMatrix* PsImage::GetParent() const
{
	return (PsMatrix*)this->parent;
}

/*
** Retrieves the absolute position of the image (see PsShape::ToAbsolute)
*/
void PsImage::GetPosition(float& x, float& y) const
{
	if (this->parent)
	{
		this->parent->ToAbsolute(this->x, this->y, x, y);
	}
	else
	{
		x = this->x;
		y = this->y;
	}
}

/*
** Changes the rotation angle of the image, with optional constraint to PI / 8.
*/
void PsImage::SetAngle(float r, bool constrain, bool)
{
	if (constrain)
		r = (float)(r / ((float)PS_MATH_PI / 8.0f)) * ((float)PS_MATH_PI / 8.0f);

	this->r = r;
}

/*
** Changes the position of the image (see PsShape::ToRelative)
*/
void PsImage::SetPosition(float x, float y, bool constrain)
{
	const PsMatrix* matrix;
	float rx;
	float ry;

	if (this->parent)
	{
		this->parent->ToRelative(x, y, rx, ry);

		if (constrain && (matrix = dynamic_cast<const PsMatrix*>(this->parent)) && matrix->div_is_active && matrix->div_x > 0 && matrix->div_y > 0)
		{
			rx = (int)((rx + SHAPE_SIZE / matrix->div_x + SHAPE_SIZE) * matrix->div_x / SHAPE_SIZE / 2) * SHAPE_SIZE * 2 / matrix->div_x - SHAPE_SIZE;
			ry = (int)((ry + SHAPE_SIZE / matrix->div_y + SHAPE_SIZE) * matrix->div_y / SHAPE_SIZE / 2) * SHAPE_SIZE * 2 / matrix->div_y - SHAPE_SIZE;
		}

		if (rx < -SHAPE_SIZE)
			rx = -SHAPE_SIZE;
		else if (rx > SHAPE_SIZE)
			rx = SHAPE_SIZE;

		if (ry < -SHAPE_SIZE)
			ry = -SHAPE_SIZE;
		else if (ry > SHAPE_SIZE)
			ry = SHAPE_SIZE;

		PsShape::FinalizePosition(rx, ry);
	}
	else
		PsShape::FinalizePosition(x, y);
}

/*
** Changes the size of the image, with optional constraint on the ratios. See the function
** "PsShape::FinalizeSize" to know what the parameters inv_x and inv_y are for, and the function
** "PsMatrix::SetSize" for the parameters old_w and old_h.
** FIXME: It might be useful to resize the image based on the size of the matrix that contains it
** (to avoid having an oversized image relative to the matrix).
*/
void PsImage::SetSize(float w, float h, float inv_x, float inv_y, float old_w, float old_h, bool constrain, bool)
{
	float ratio_h;
	float ratio_w;

	if ((constrain || this->constraint) && old_w && old_h)
	{
		ratio_h = h / old_h;
		ratio_w = w / old_w;

		h = old_h * (ratio_h + ratio_w) / 2;
		w = old_w * (ratio_h + ratio_w) / 2;
	}

	// FIXME: Limit the maximum size of the image

	PsShape::FinalizeSize(w, h, inv_x, inv_y);
}

/*
** Changes the torsion of the image (impossible operation, so no effect)
*/
void PsImage::SetTorsion(float, float, bool)
{
}

/*
** Loads a texture from a buffer (used when loading a file). Due to the way calculations are performed
** subsequently, the size of an image from PatternShop's point of view must be half its actual size.
*/
bool PsImage::TextureFromBuffer(uint8* buffer, bool resize)
{
	int x;
	int y;

	if (!this->texture.LoadFromBuffer(buffer))
		return false;

	if (resize)
	{
		this->texture.GetSize(x, y);
		this->SetSize(x / 2.0f, y / 2.0f);
	}

	return true;
}

/*
** Loads a texture from a file.
*/
bool PsImage::TextureFromFile(const char* file, bool resize)
{
	int x;
	int y;

	if (!this->texture.LoadFromFile(file))
		return false;

	if (resize)
	{
		this->texture.GetSize(x, y);
		this->SetSize((float)x, (float)y);
	}

	return true;
}
