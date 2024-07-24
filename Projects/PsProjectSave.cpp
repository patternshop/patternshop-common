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
#include "PsProjectSave.h"

PsProjectSave::PsProjectSave(PsProjectController& project) : project(project), file(NULL)
{
}

PsProjectSave::~PsProjectSave()
{
}

/*
** Saves a project to a file
*/
ErrID PsProjectSave::saveProject(const char* path)
{
	ErrID err;
	size_t n;
	int x;
	int y;

	if (!(this->file = fopen(path, "wb")))
		return ERROR_FILE_ACCESS;

	n = ACCEPTED_ENDIAN_FILE;
	fwrite(&n, sizeof(size_t), 1, this->file);

	n = PROJECT_VERSION;
	fwrite(&n, sizeof(size_t), 1, this->file);

	this->project.renderer.GetDocSize(x, y);
	fwrite(&x, sizeof(int), 1, this->file);
	fwrite(&y, sizeof(int), 1, this->file);

	n = this->project.images.size();

	if (fwrite(&n, sizeof(size_t), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	for (ImageList::const_iterator i = this->project.images.begin(); i != this->project.images.end(); ++i)
		if ((err = this->saveImage(**i)) != ERROR_NONE)
			return err;

	n = this->project.matrices.size();
	fwrite(&n, sizeof(size_t), 1, this->file);

	for (MatrixList::const_iterator i = this->project.matrices.begin(); i != this->project.matrices.end(); ++i)
		if ((err = this->saveMatrix(**i)) != ERROR_NONE)
		{
			fclose(this->file);
			return err;
		}

	bool pattern_exist = false;
	if (this->project.pattern) pattern_exist = true;
	fwrite(&pattern_exist, sizeof(pattern_exist), 1, this->file);
	if (pattern_exist)
	{
		if ((err = this->savePattern(*(this->project.pattern))) != ERROR_NONE)
		{
			fclose(this->file);
			return err;
		}
	}

	fwrite(&this->project.bHideColor, sizeof(this->project.bHideColor), 1, this->file);
	fwrite(&this->project.iColor, sizeof(this->project.iColor), 1, this->file);

	fclose(this->file);

	this->project.bNeedSave = false;

	return ERROR_NONE;
}

/*
** Saves the data of a PsShape to an open file.
*/
ErrID PsProjectSave::saveShape(PsShape& shape) const
{
	if (fwrite(&shape.h, sizeof(shape.h), 1, this->file) != 1 ||
		fwrite(&shape.i, sizeof(shape.i), 1, this->file) != 1 ||
		fwrite(&shape.j, sizeof(shape.j), 1, this->file) != 1 ||
		fwrite(&shape.r, sizeof(shape.r), 1, this->file) != 1 ||
		fwrite(&shape.w, sizeof(shape.w), 1, this->file) != 1 ||
		fwrite(&shape.x, sizeof(shape.x), 1, this->file) != 1 ||
		fwrite(&shape.y, sizeof(shape.y), 1, this->file) != 1 ||
		fwrite(&shape.hide, sizeof(shape.hide), 1, this->file) != 1 ||
		fwrite(&shape.constraint, sizeof(shape.constraint), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	return ERROR_NONE;
}

/*
** Saves the image data to a file.
*/
ErrID PsProjectSave::saveImage(PsImage& image) const
{
	uint8* buffer;
	size_t size;

	if (!(buffer = image.texture.GetBuffer(size)))
		return ERROR_FILE_WRITE;

	if (fwrite(&size, sizeof(size), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(buffer, sizeof(*buffer), size, this->file) != size)
		return ERROR_FILE_WRITE;

	return this->saveShape(image);
}

/*
** Saves all the data of a matrix to a file.
*/
ErrID PsProjectSave::saveMatrix(PsMatrix& matrix) const
{
	ImageList::const_iterator	i;
	ErrID						err;
	size_t						n;

	if (fwrite(&matrix.color, sizeof(int), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	n = matrix.images.size();

	if (fwrite(&n, sizeof(n), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	for (i = matrix.images.begin(); i != matrix.images.end(); ++i)
		if ((err = this->saveImage(**i)) != ERROR_NONE)
			return err;

	if (fwrite(&matrix.div_is_active, sizeof(matrix.div_is_active), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&matrix.div_x, sizeof(matrix.div_x), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&matrix.div_y, sizeof(matrix.div_y), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	return this->saveShape(matrix);
}

/*
** Saves all the data of a layer to a file.
*/
ErrID PsProjectSave::saveLayer(PsLayer& layer) const
{
	if (fwrite(&layer.iWidth, sizeof(layer.iWidth), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&layer.iHeight, sizeof(layer.iHeight), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(layer.ucData, sizeof(*layer.ucData), layer.iWidth * layer.iHeight, this->file) != layer.iWidth * layer.iHeight)
		return ERROR_FILE_WRITE;

	if (fwrite(&layer.vTranslation, sizeof(PsVector), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&layer.rRotation, sizeof(PsRotator), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&layer.fScale, sizeof(layer.fScale), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	return ERROR_NONE;
}

/**
 * Save the pattern content to a file.
*/
ErrID PsProjectSave::savePattern(PsPattern& pattern) const
{
	uint8* buffer;
	size_t size;

	if (!(buffer = pattern.texture.GetBuffer(size)))
		return ERROR_FILE_WRITE;

	if (fwrite(&size, sizeof(size), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(buffer, sizeof(*buffer), size, this->file) != size)
		return ERROR_FILE_WRITE;

	if (fwrite(&pattern.hide, sizeof(pattern.hide), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&pattern.light_power, sizeof(pattern.light_power), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&pattern.light_range, sizeof(pattern.light_range), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	size = pattern.aLayers.size();
	if (fwrite(&size, sizeof(size_t), 1, this->file) != 1)
		return ERROR_FILE_WRITE;

	for (int i = 0; i < pattern.aLayers.size(); ++i)
		if (this->saveLayer(*(pattern.aLayers[i])) != ERROR_NONE)
			return ERROR_FILE_WRITE;

	return ERROR_NONE;
}
