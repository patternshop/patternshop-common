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
** Une image est toujours contenue dans une matrice, elle utilise donc le constructeur
** correspondant de l'PsShape.
*/
PsImage::PsImage(PsMatrix* parent) :
	PsShape(*parent)
{
}

PsImage::~PsImage()
{
}

/*
** Retourne la matrice dans laquelle est contenue l'image.
*/
PsMatrix* PsImage::GetParent() const
{
	return (PsMatrix*)parent;
}

/*
** Charge les données de l'image depuis un fichier.
*/
ErrID				PsImage::FileLoad(FILE* file)
{
	uint8* buffer;
	size_t			size;

	if (fread(&size, sizeof(size), 1, file) != 1)
		return ERROR_FILE_READ;

	buffer = new uint8[size];

	if (fread(buffer, sizeof(*buffer), size, file) != size)
		return ERROR_FILE_READ;

	if (!TextureFromBuffer(buffer))
		return ERROR_FILE_READ;

	return PsShape::FileLoad(file);
}

/*
** Enregistre les données de l'image dans un fichier.
*/
ErrID				PsImage::FileSave(FILE* file) const
{
	uint8* buffer;
	size_t			size;

	if (!(buffer = texture.GetBuffer(size)))
		return ERROR_FILE_WRITE;

	if (fwrite(&size, sizeof(size), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(buffer, sizeof(*buffer), size, file) != size)
		return ERROR_FILE_WRITE;

	return PsShape::FileSave(file);
}

/*
** Récupere la position absolue de l'image (voir PsShape::ToAbsolute)
*/
void	PsImage::GetPosition(float& x, float& y) const
{
	if (parent)
	{
		parent->ToAbsolute(this->x, this->y, x, y);
	}
	else
	{
		x = this->x;
		y = this->y;
	}
}

/*
** Change l'angle de rotation de l'image, avec éventuelle contrainte à PI / 8.
*/
void	PsImage::SetAngle(float r, bool constrain, bool)
{
	if (constrain)
		r = (int)(r / (PS_MATH_PI / 8)) * (PS_MATH_PI / 8);

	this->r = r;
}

/*
** Change la position de l'image (voir PsShape::ToRelative)
*/
void				PsImage::SetPosition(float x, float y, bool constrain)
{
	const PsMatrix* matrix;
	float			rx;
	float			ry;

	if (this->parent)
	{
		parent->ToRelative(x, y, rx, ry);

		if (constrain && (matrix = dynamic_cast<const PsMatrix*> (this->parent)) && matrix->div_is_active && matrix->div_x > 0 && matrix->div_y > 0)
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
** Change la taille de l'image, avec éventuelle contrainte sur les ratios. Voir la fonction
** "PsShape::FinalizeSize" pour savoir à quoi servent les paramètres inv_x et inv_y, et la fonction
** "PsMatrix::SetSize" pour les paramètres old_w et old_h.
** FIXME : Il serait peut-être utile de redimentionner l'image en fonction de la taille
** de la matrice qui la contient (afin de ne pas avoir d'image démesurée par rapport à la
** matrice).
*/
void		PsImage::SetSize(float w, float h, float inv_x, float inv_y, float old_w, float old_h, bool constrain, bool)
{
	float	ratio_h;
	float	ratio_w;

	if ((constrain || this->constraint) && old_w && old_h)
	{
		ratio_h = h / old_h;
		ratio_w = w / old_w;

		h = old_h * (ratio_h + ratio_w) / 2;
		w = old_w * (ratio_h + ratio_w) / 2;
	}

	// FIXME : Limiter la taille maximum de l'image

	PsShape::FinalizeSize(w, h, inv_x, inv_y);
}

/*
** Change la torsion de l'image (opération impossible, donc sans effet)
*/
void	PsImage::SetTorsion(float, float, bool)
{
}

/*
** Charge une texture depuis un buffer (utilisé lors du chargement d'un fichier). À cause
** de la façon dont les calculs sont effectués par la suite, La taille d'une image du point
** de vue de PatternShop doit être la moitié de sa taille réelle.
*/
bool	PsImage::TextureFromBuffer(uint8* buffer, bool resize)
{
	int	x;
	int	y;

	if (!texture.LoadFromBuffer(buffer))
		return false;

	if (resize)
	{
		texture.GetSize(x, y);
		this->SetSize(x / 2.0f, y / 2.0f);
	}

	return true;
}

/*
** Charge une texture depuis un fichier.
*/
bool	PsImage::TextureFromFile(const char* file, bool resize)
{
	int	x;
	int	y;

	if (!texture.LoadFromFile(file))
		return false;

	if (resize)
	{
		texture.GetSize(x, y);
		this->SetSize(x, y);
	}

	return true;
}
