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
#pragma once

#include <stdio.h>
#include <list>
#include "PsShape.h"
#include "PsImage.h"
#include "PsMessage.h"

#define MATRIX_COUNT_COLOR	6

#define MATRIX_ROUND_TORSIO	(64.0f)

class	Project;
class	PsRender;

typedef std::list<PsImage*> ImageList;

/*
** Une matrice est une figure géometrique qui n'est pas contenue dans une autre. Voir
** la classe PsShape avant de lire celle-ci.
*/
class	PsMatrix : public PsShape
{
	friend class	Project;
	friend class	PsRender;

public:
	PsMatrix();
	virtual			~PsMatrix();

	virtual void	DoResetAll();
	virtual void	DoChangeColor();

	virtual void	GetColor(float&, float&, float&) const;
	virtual void	GetPosition(float&, float&) const;
	virtual void	GetTorsion(float&, float&) const;

	virtual void	SetAngle(float, bool = false, bool = false);
	virtual void	SetPosition(float, float, bool = false);
	virtual void	SetSize(float, float, float = 0, float = 0, float = 0, float = 0, bool = false, bool = false);
	virtual void	SetTorsion(float, float, bool = false);

	virtual bool	HasTorsion();

public:
	static int			current_color;
	static int			default_w, default_h, minimum_dim;

	ImageList			images;
	int					color;
	bool				div_is_active;
	int					div_x, div_y;
};
