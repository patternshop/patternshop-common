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
#include "PsShape.h"
#include "PsMaths.h"

/*
** Constructeur pour une PsShape contenue dans une autre.
*/
PsShape::PsShape (const PsShape& parent) :
	parent (&parent),
	h (200),
	i (0),
	j (0),
	r (0),
	w (200),
	x (0),
	y (0),
	hide (false),
	constraint (false)
{
}

/*
** Constructeur pour une PsShape autonome.
*/
PsShape::PsShape() :
	parent (0),
	h (0),
	i (0),
	j (0),
	r (0),
	w (0),
	x (0),
	y (0),
	hide (false),
	constraint (false)
{
}

PsShape::~PsShape()
{
}

/*
** Récupere l'angle de rotation de l'PsShape.
*/
float	PsShape::GetAngle() const
{
	return this->r;
}

/*
** Récupere le nom de cette PsShape (techniquement inutile, mais ce nom sera affiché
** dans les boites de dialogue, pour l'utilisateur).
*/
const std::string&	PsShape::GetName() const
{
	return this->name;
}

/*
** Charge les données d'une PsShape depuis un fichier ouvert.
** FIXME : Le nom n'est pas chargé (ni sauvegardé)
*/
ErrID	PsShape::FileLoad (FILE* file)
{
	if (	fread (&h, sizeof (h), 1, file) != 1 ||
			fread (&i, sizeof (i), 1, file) != 1 ||
			fread (&j, sizeof (j), 1, file) != 1 ||
			fread (&r, sizeof (r), 1, file) != 1 ||
			fread (&w, sizeof (w), 1, file) != 1 ||
			fread (&x, sizeof (x), 1, file) != 1 ||
			fread (&y, sizeof (y), 1, file) != 1 ||
			fread (&hide, sizeof (hide), 1, file) != 1 ||
			fread (&constraint, sizeof (constraint), 1, file) != 1)
		return ERROR_FILE_READ;

	return ERROR_NONE;
}

/*
** Enregistre les données d'une PsShape dans un fichier ouvert.
** FIXME : Le nom n'est pas chargé (ni sauvegardé)
*/
ErrID	PsShape::FileSave (FILE* file) const
{
	if (	fwrite (&h, sizeof (h), 1, file) != 1 ||
			fwrite (&i, sizeof (i), 1, file) != 1 ||
			fwrite (&j, sizeof (j), 1, file) != 1 ||
			fwrite (&r, sizeof (r), 1, file) != 1 ||
			fwrite (&w, sizeof (w), 1, file) != 1 ||
			fwrite (&x, sizeof (x), 1, file) != 1 ||
			fwrite (&y, sizeof (y), 1, file) != 1 ||
			fwrite (&hide, sizeof (hide), 1, file) != 1 ||
			fwrite (&constraint, sizeof (constraint), 1, file) != 1)
		return ERROR_FILE_WRITE;

	return ERROR_NONE;
}

/*
** Change la position de l'PsShape. Cette méthode qui devrait s'appeller "SetPosition" porte le préfixe
** "Finalize", qui indique qu'un autre traitement sera nécessaire par la classe qui héritera d'PsShape,
** avant d'appeler cette méthode, et que son appel seul ne suffit donc pas.
*/
void	PsShape::FinalizePosition (float x, float y)
{
	this->x = x;
	this->y = y;
}

/*
** Change la taille de l'PsShape, en largeur (w) et hauteur (h). Le point "inv_x, inv_y" est
** le "point invariant" : celui qui ne doit pas bouger lors du redimentionnement. Quand
** l'utilisateur redimentionne une PsShape, ce point est la poignée opposée à celle qui est
** tirée actuellement. Si inv_x et inv_y restent à 0, l'PsShape change simplement de taille
** sans compenser aucun déplacement.
*/
void		PsShape::FinalizeSize (float w, float h, float inv_x, float inv_y)
{
	float	inv_x1;
	float	inv_x2;
	float	inv_y1;
	float	inv_y2;

	ToAbsolute (inv_x, inv_y, inv_x1, inv_y1);

	this->h = h;
	this->w = w;

	if (inv_x || inv_y)
	{
		ToAbsolute (inv_x, inv_y, inv_x2, inv_y2);

		inv_x2 -= inv_x1;
		inv_y2 -= inv_y1;

		GetPosition (inv_x1, inv_y1);
		SetPosition (inv_x1 + inv_x2, inv_y1 + inv_y2);
	}
}

/*
** Teste si un point (en absolu, par rapport au document et non à la fenêtre, cf fonction
** PsRender::Convert) est dans une PsShape ou non.
*/
bool		PsShape::InContent (float ax, float ay) const
{
	float	rx, ry;

	ToRelative (ax, ay, rx, ry);

	return rx >= -SHAPE_SIZE && rx <= SHAPE_SIZE && ry >= -SHAPE_SIZE && ry <= SHAPE_SIZE;
}

/*
** Teste si un point est sur l'une des poignées de redimentionnement d'une PsShape, et enregistre
** dans "i" l'index de la poignée (0 = haut/gauche, etc dans le sens horaire). Cette fonction doit
** prendre le zoom en paramètre, pour que la taille des poignées ne soient pas affectées par lui
** (on doit donc pouvoir annuler ses effets).
*/
bool		PsShape::InResize (float px, float py, float zoom, int& i) const
{
	float	size = SHAPE_SIZE_RESIZE * zoom;
	float	corner[4][2];

	ToAbsolute (-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	ToAbsolute (SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	ToAbsolute (-SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	ToAbsolute (SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

	for (i = 0; i < 4; ++i)
		if (px >= corner[i][0] - size && px <= corner[i][0] + size && py >= corner[i][1] - size && py <= corner[i][1] + size)
			return true;

	return false;
}

/*
** Teste si un point est à l'un des emplacements de rotation; même remarques que pour
** le redimentionnement.
*/
bool		PsShape::InRotate (float px, float py, float zoom, int& i) const
{
	float	size = SHAPE_SIZE_ROTATE * zoom;
	float	corner[4][2];
	float	angle;
	float	ax;
	float	ay;

	ToAbsolute (-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	ToAbsolute (SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	ToAbsolute (-SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	ToAbsolute (SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

	for (i = 0; i < 4; ++i)
	{
		angle = ToAngle (corner[i][0], corner[i][1]);

		ax = size * cos (angle) - size * sin (angle);
		ay = size * sin (angle) + size * cos (angle);

		if (px >= corner[i][0] + ax - size && px <= corner[i][0] + ax + size && py >= corner[i][1] + ay - size && py <= corner[i][1] + ay + size)
			return true;
	}

	return false;
}

/*
** Teste si un point est sur l'une des poignées de torsion, toujours les mêmes remarques
** que pour les deux fonctions précedentes.
*/
bool	PsShape::InTorsion (float px, float py, float zoom, int& i) const
{
	float	size = SHAPE_SIZE_TORSIO * zoom;
	float	corner[4][2];

	ToAbsolute (0, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	ToAbsolute (SHAPE_SIZE, 0, corner[1][0], corner[1][1]);
	ToAbsolute (0, SHAPE_SIZE, corner[2][0], corner[2][1]);
	ToAbsolute (-SHAPE_SIZE, 0, corner[3][0], corner[3][1]);

	for (i = 0; i < 4; ++i)
		if (px >= corner[i][0] - size && px <= corner[i][0] + size && py >= corner[i][1] - size && py <= corner[i][1] + size)
			return true;

	return false;
}

/*
** Change le nom de l'PsShape (voir GetName).
*/
void	PsShape::SetName (const std::string& name)
{
	this->name = name;
}

/*
** Transforme les coordonnées relatives de l'PsShape en coordonnées absolues. Les coordonnées
** relatives permettent des calculs bien plus simples à l'interieur d'une PsShape (son coin
** haut/gauche est toujours à -SHAPE_SIZE, -SHAPE_SIZE quelles que soient les transformations
** appliquées, par exemple). Les coordonnées absolues prennent en compte ces transformations.
*/
void		PsShape::ToAbsolute (float rx, float ry, float& ax, float& ay) const
{
	float	sx;
	float	sy;
	float	tx;
	float	ty; 

	tx = rx * w / (SHAPE_SIZE * 2) + i * ry / (SHAPE_SIZE * 2);
	ty = ry * h / (SHAPE_SIZE * 2) + j * rx / (SHAPE_SIZE * 2);

	if (parent)
		parent->ToAbsolute (x, y, sx, sy);
	else
	{
		sx = x;
		sy = y;
	}

	ax = tx * cos (r) - ty * sin (r) + sx;
	ay = tx * sin (r) + ty * cos (r) + sy;
}

/*
** Retourne l'angle entre le centre de l'PsShape et le point "ax, ay", en radians.
*/
float		PsShape::ToAngle (float ax, float ay) const
{
	float	tx;
	float	ty;

	if (parent)
		parent->ToAbsolute (x, y, tx, ty);
	else
	{
		tx = x;
		ty = y;
	}

	if (ax < tx)
		return (float)atan ((ay - ty) / (ax - tx)) + PS_MATH_PI;
	else if (ax > tx)
		return (float)atan ((ay - ty) / (ax - tx));
	else
		return (ay < ty ? -PS_MATH_PI : PS_MATH_PI) / 2.0f;
}

/*
** Conversion de coordonnées absolues (donc au niveau du document) en coordonnées relatives
** à l'PsShape (voir ToAbsolute).
*/
void		PsShape::ToRelative (float ax, float ay, float& rx, float& ry) const
{
	float	sx;
	float	sy;
	float	tx;
	float	ty;

	if (parent)
		parent->ToAbsolute (x, y, sx, sy);
	else
	{
		sx = x;
		sy = y;
	}

	sx = ax - sx;
	sy = ay - sy;

	tx = (sx * cos (-r) - sy * sin (-r));
	ty = (sx * sin (-r) + sy * cos (-r));

	rx = (tx - i * ty / h) / (w - j / h) * (SHAPE_SIZE * 2);
	ry = (ty - j * tx / w) / (h - i / w) * (SHAPE_SIZE * 2);
}
