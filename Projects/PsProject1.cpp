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
** Initially, no pattern, matrix, image, etc. is selected.
*/
PsProjectController::PsProjectController() :
	pattern(0),
	matrix(0),
	image(0),
	shape(0),
	center(true),
	log_insert(false)
{
	this->iColor[0] = 255;
	this->iColor[1] = 255;
	this->iColor[2] = 255;
	this->bHideColor = false;
	this->bNeedSave = false;
	this->iLayerId = 0;
	this->bPatternsIsSelected = false;
}

PsProjectController::~PsProjectController()
{
	this->LogFlush();
}

/*
** Duplicates the currently selected image
*/
ErrID PsProjectController::CloneImage()
{
	uint8* buffer1;
	uint8* buffer2;
	PsImage* image;
	size_t size;

	if (!this->image)
		return ERROR_IMAGE_SELECT;

	image = new PsImage(this->matrix);

	buffer1 = this->image->GetTexture().GetBuffer(size);
	buffer2 = new uint8[size];
	memcpy(buffer2, buffer1, size * sizeof(*buffer2));

	if (!image->TextureFromBuffer(buffer2))
		return ERROR_IMAGE_LOAD;

	image->SetAngle(this->image->GetAngle());
	image->SetSize(this->image->w, this->image->h);
	image->SetTorsion(this->image->i, this->image->j);

	if (this->matrix)
	{
		this->matrix->images.push_back(image);
		image->x = 0;
		image->y = 0;
	}
	else
	{
		this->images.push_back(image);
		image->SetPosition(this->GetWidth() / 2.0f, this->GetHeight() / 2.0f);
	}

	this->LogAdd(new LogNewImage(*this, image, true));
	this->SelectImage(image);

	return ERROR_NONE;
}

/*
** Duplicates the currently selected matrix
*/
ErrID PsProjectController::CloneMatrix()
{
	uint8* buffer1;
	uint8* buffer2;
	PsMatrix* matrix;
	ImageList::iterator ti;
	PsImage* image;
	size_t size;

	if (!this->matrix)
		return ERROR_MATRIX_SELECT;

	matrix = new PsMatrix();

	this->matrices.push_back(matrix);

	matrix->SetAngle(this->matrix->GetAngle());
	matrix->SetSize(this->matrix->w, this->matrix->h);
	matrix->SetTorsion(this->matrix->i, this->matrix->j);
	matrix->SetPosition(this->GetWidth() / 2.0f, this->GetHeight() / 2.0f);

	this->LogAdd(new LogNewMatrix(*this, matrix, true));

	for (ti = this->matrix->images.begin(); ti != this->matrix->images.end(); ++ti)
	{
		image = new PsImage(matrix);

		buffer1 = (*ti)->GetTexture().GetBuffer(size);
		buffer2 = new uint8[size];
		memcpy(buffer2, buffer1, size * sizeof(*buffer2));

		if (!image->TextureFromBuffer(buffer2))
			return ERROR_IMAGE_LOAD;

		matrix->images.push_back(image);
		image->SetAngle((*ti)->GetAngle());
		image->SetSize((*ti)->w, (*ti)->h);
		image->SetTorsion((*ti)->i, (*ti)->j);
		image->x = (*ti)->x;
		image->y = (*ti)->y;

		this->LogAdd(new LogNewImage(*this, image, true));
	}

	this->SelectMatrix(matrix);

	return ERROR_NONE;
}

/*
** Replaces the currently selected image
*/
ErrID PsProjectController::ReplaceImage(const char* file)
{
	if (!this->image)
		return ERROR_IMAGE_SELECT;

	this->LogAdd(new LogReplace(*this, this->image));

	if (!this->image->TextureFromFile(file, false))
		return ERROR_IMAGE_LOAD;

	return ERROR_NONE;
}

/*
** Deletes the selected image from the matrix that contains it.
*/
ErrID PsProjectController::DelImage()
{
	ImageList::iterator ti;
	MatrixList::iterator tm;

	if (!this->image)
		return ERROR_IMAGE_SELECT;

	this->LogAdd(new LogDelImage(*this, this->image, false));

	if (!this->image->parent)
		this->images.remove(this->image);
	else
		this->matrix->images.remove(this->image);

	delete this->image;

	PsMatrix* b = this->matrix;
	this->SelectImage(0);
	this->matrix = b;

	return ERROR_NONE;
}

/*
** Deletes the selected matrix and selects another one if any remain.
*/
ErrID PsProjectController::DelMatrix()
{
	ImageList::iterator t;

	if (!this->matrix)
		return ERROR_MATRIX_SELECT;

	for (t = this->matrix->images.begin(); t != this->matrix->images.end(); ++t)
		this->LogAdd(new LogDelImage(*this, *t, false));

	this->LogAdd(new LogDelMatrix(*this, this->matrix, false));

	this->matrices.remove(this->matrix);
	delete this->matrix;

	this->SelectMatrix(0);

	return ERROR_NONE;
}

/*
** Deletes the selected pattern and selects another one if any remain.
*/
ErrID PsProjectController::DelPattern()
{
	if (!this->pattern)
		return ERROR_PATTERN_SELECT;

	delete this->pattern;
	this->pattern = 0;

	return ERROR_NONE;
}

/*
** Imports a new image outside of a matrix.
*/
ErrID PsProjectController::NewImage(const char* file)
{
	PsImage* image;

	image = new PsImage(NULL);

	if (!image->TextureFromFile(file))
		return ERROR_IMAGE_LOAD;

	image->SetPosition(this->GetWidth() / 2.0f, this->GetHeight() / 2.0f);
	this->images.push_back(image);

	this->SelectImage(image);

	this->LogAdd(new LogNewImage(*this, image, true));

	return ERROR_NONE;
}

/*
** Imports a new image into the current matrix, if there is one.
*/
ErrID PsProjectController::NewMotif(const char* file)
{
	PsImage* image;
	float x;
	float y;

	if (!this->matrix)
	{
		if (this->matrices.size() == 0)
			this->NewMatrix();

		this->matrix = *this->matrices.begin();
	}

	image = new PsImage(this->matrix);

	if (!image->TextureFromFile(file))
		return ERROR_IMAGE_LOAD;

	this->matrix->images.push_back(image);
	this->matrix->GetPosition(x, y);
	image->SetPosition(x, y);

	this->SelectImage(image);

	this->LogAdd(new LogNewImage(*this, image, true));

	return ERROR_NONE;
}

/*
** Creates a new matrix in the document. Its initial size is currently a quarter
** of that of the document, but this is completely arbitrary (yes, a quarter and not an eighth,
** the remark of PsImage::TextureFromBuffer also applies here).
*/
ErrID PsProjectController::NewMatrix()
{
	this->matrix = new PsMatrix();
	this->image = 0;

	this->matrices.push_back(this->matrix);
	this->matrix->SetPosition(this->GetWidth() / 2.0f, this->GetHeight() / 2.0f);
	this->matrix->SetSize((float)PsMatrix::default_w, (float)PsMatrix::default_h);

	this->SelectMatrix(this->matrix);

	this->LogAdd(new LogNewMatrix(*this, this->matrix, true));

	return ERROR_NONE;
}

/*
** Creates a new pattern in the document.
*/
ErrID PsProjectController::NewPattern(const char* file)
{
	if (this->pattern)
		delete this->pattern;

	this->pattern = new PsPattern();

	if (!this->pattern->TextureFromFile(file))
	{
		delete this->pattern;
		this->pattern = 0;
		return ERROR_PATTERN_LOAD;
	}

	//-- initial setup
	for (int i = 0; i < this->pattern->GetChannelsCount(); ++i)
	{
		PsLayer* layer = this->pattern->aLayers[i];
		layer->vTranslation.X = this->GetWidth() / 2.f;
		layer->vTranslation.Y = this->GetHeight() / 2.f;
	}
	//--

	this->pattern->UpdateScale(this->GetWidth(), this->GetHeight());

	this->pattern = this->pattern;
	this->iLayerId = 0;

	return ERROR_NONE;
}
