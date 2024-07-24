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
#include "PsProjectLoad.h"

PsProjectLoad::PsProjectLoad(PsProject& project) : project(project), file(NULL)
{
}

PsProjectLoad::~PsProjectLoad()
{
}

/**
 * Loads the project from a file. The constant macro "PROJECT_VERSION" is used to check the version
 * of the save and thus serves as a magic number.
 */
ErrID PsProjectLoad::loadProject(const char* path)
{
	PsMatrix* matrix;
	ErrID err;
	size_t n;
	int x;
	int y;

	if (!(this->file = fopen(path, "rb")))
		return ERROR_FILE_ACCESS;

	PsController::Instance().SetProgress(-1);

	this->project.LogFlush();

	fread(&n, sizeof(size_t), 1, this->file);

	if (n != ACCEPTED_ENDIAN_FILE)
	{
		PsController::Instance().SetProgress(-2);
		return ERROR_FILE_VERSION;
	}

	fread(&n, sizeof(size_t), 1, this->file);

	if (n != PROJECT_VERSION && n != PROJECT_VERSION_OLD_BETA) // FIXME
	{
		PsController::Instance().SetProgress(-2);
		return ERROR_FILE_VERSION;
	}

	fread(&x, sizeof(x), 1, this->file);
	fread(&y, sizeof(y), 1, this->file);
	this->project.renderer.SetDocSize(x, y);

	PsController::Instance().SetProgress(10);

	if (n != PROJECT_VERSION_OLD_BETA) // FIXME
	{
		if (fread(&n, sizeof(n), 1, this->file) != 1)
		{
			PsController::Instance().SetProgress(-2);
			return ERROR_FILE_READ;
		}
		while (n--)
		{
			this->project.images.push_back((this->project.image = new PsImage(NULL)));

			if ((err = this->loadImage(*(this->project.image))) != ERROR_NONE)
			{
				PsController::Instance().SetProgress(-2);
				return err;
			}
		}
	}

	fread(&n, sizeof(n), 1, this->file);

	for (uint32 i = 0; i < n; ++i)
	{
		PsController::Instance().SetProgress(10 + 90 * i / n);
		matrix = new PsMatrix();
		this->project.matrices.push_back(matrix);

		if ((err = this->loadMatrix(*matrix)) != ERROR_NONE)
		{
			PsController::Instance().SetProgress(-2);
			return err;
		}
	}

	bool pattern_exist = false;
	fread(&pattern_exist, sizeof(pattern_exist), 1, this->file);
	if (pattern_exist)
	{
		this->project.pattern = new PsPattern();
		if ((err = this->loadPattern(*this->project.pattern)) != ERROR_NONE)
		{
			PsController::Instance().SetProgress(-2);
			return err;
		}
		this->project.pattern->UpdateScale(this->project.GetWidth(), this->project.GetHeight());
	}

	fread(&this->project.bHideColor, sizeof(this->project.bHideColor), 1, this->file);
	fread(&this->project.iColor, sizeof(this->project.iColor), 1, this->file);

	fclose(this->file);

	this->project.center = true;

	PsController::Instance().SetProgress(-2);

	return ERROR_NONE;
}

/*
** Loads the data of a PsShape from an open file.
*/
ErrID PsProjectLoad::loadShape(PsShape& shape) const
{
	if (fread(&shape.h, sizeof(shape.h), 1, this->file) != 1 ||
		fread(&shape.i, sizeof(shape.i), 1, this->file) != 1 ||
		fread(&shape.j, sizeof(shape.j), 1, this->file) != 1 ||
		fread(&shape.r, sizeof(shape.r), 1, this->file) != 1 ||
		fread(&shape.w, sizeof(shape.w), 1, this->file) != 1 ||
		fread(&shape.x, sizeof(shape.x), 1, this->file) != 1 ||
		fread(&shape.y, sizeof(shape.y), 1, this->file) != 1 ||
		fread(&shape.hide, sizeof(shape.hide), 1, this->file) != 1 ||
		fread(&shape.constraint, sizeof(shape.constraint), 1, this->file) != 1)
		return ERROR_FILE_READ;

	return ERROR_NONE;
}

/*
** Loads the image data from a file.
*/
ErrID PsProjectLoad::loadImage(PsImage& image) const
{
	uint8* buffer;
	size_t size;

	if (fread(&size, sizeof(size), 1, this->file) != 1)
		return ERROR_FILE_READ;

	buffer = new uint8[size];

	if (fread(buffer, sizeof(*buffer), size, this->file) != size)
		return ERROR_FILE_READ;

	if (!image.TextureFromBuffer(buffer))
		return ERROR_FILE_READ;

	return this->loadShape(image);
}

/*
** Loads the data of a matrix (this includes all the images it contains) from a file.
*/
ErrID PsProjectLoad::loadMatrix(PsMatrix& matrix) const
{
	PsImage* image;
	ErrID	err;
	size_t	n;

	if (fread(&matrix.color, sizeof(matrix.color), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&n, sizeof(n), 1, this->file) != 1)
		return ERROR_FILE_READ;

	while (n--)
	{
		matrix.images.push_back((image = new PsImage(&matrix)));

		if ((err = this->loadImage(*image)) != ERROR_NONE)
			return err;
	}

	if (fread(&matrix.div_is_active, sizeof(matrix.div_is_active), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&matrix.div_x, sizeof(matrix.div_x), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&matrix.div_y, sizeof(matrix.div_y), 1, this->file) != 1)
		return ERROR_FILE_READ;

	return this->loadShape(matrix);
}

/*
** Loads the data of a layer (this includes all the images it contains) from a file.
*/
ErrID PsProjectLoad::loadLayer(PsLayer& layer) const
{

	if (fread(&layer.iWidth, sizeof(layer.iWidth), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&layer.iHeight, sizeof(layer.iHeight), 1, this->file) != 1)
		return ERROR_FILE_READ;

	layer.ucData = new uint8[layer.iWidth * layer.iHeight];
	if (fread(layer.ucData, sizeof(*layer.ucData), layer.iWidth * layer.iHeight, this->file) != layer.iWidth * layer.iHeight)
		return ERROR_FILE_READ;

	if (layer.Register(layer.iWidth, layer.iHeight, layer.ucData) != ERROR_NONE)
		return ERROR_FILE_READ;

	if (fread(&layer.vTranslation, sizeof(PsVector), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&layer.rRotation, sizeof(PsRotator), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&layer.fScale, sizeof(layer.fScale), 1, this->file) != 1)
		return ERROR_FILE_READ;

	return ERROR_NONE;
}

/**
 * Load the pattern content from a file.
*/
ErrID PsProjectLoad::loadPattern(PsPattern& pattern) const
{
	uint8* buffer;
	size_t size;

	if (fread(&size, sizeof(size), 1, this->file) != 1)
		return ERROR_FILE_READ;

	buffer = new uint8[size];

	if (fread(buffer, sizeof(*buffer), size, this->file) != size)
		return ERROR_FILE_READ;

	if (fread(&pattern.hide, sizeof(pattern.hide), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&pattern.light_power, sizeof(pattern.light_power), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (fread(&pattern.light_range, sizeof(pattern.light_range), 1, this->file) != 1)
		return ERROR_FILE_READ;

	if (!pattern.TextureFromBuffer(buffer))
		return ERROR_FILE_READ;

	size_t iLayerCount;
	if (fread(&iLayerCount, sizeof(size_t), 1, this->file) != 1)
		return ERROR_FILE_READ;

	for (int i = 0; i < iLayerCount; ++i)
	{
		PsLayer* layer = new PsLayer;
		if (this->loadLayer(*layer) != ERROR_NONE)
		{
			delete layer;
			return ERROR_FILE_WRITE;
		}
		pattern.aLayers.push_back(layer);
	}
	return ERROR_NONE;
}
