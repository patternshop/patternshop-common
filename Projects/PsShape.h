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

#include <string>
#include <math.h>
#include "PsMessage.h"

#define	SHAPE_SIZE			(512.0f)

#define	SHAPE_SIZE_RESIZE	(4.0f)
#define	SHAPE_SIZE_ROTATE	(16.0f)
#define SHAPE_SIZE_TORSIO	(4.0f)

#define SHAPE_SPACE_ROTATE	(16.0f)

class	PsRender;

/*
** PsShape est une figure géometrique, donc soit une image soit une matrice dans notre cas.
** Elle peut être "contenue" dans une autre PsShape (les images dans les shapes), ou bien
** être autonome (les shapes).
*/
class	PsShape
{
	friend class	PsRender;

public:
	PsShape(const PsShape&);
	PsShape();
	virtual ~PsShape();

	virtual float	GetAngle() const;
	virtual const	std::string& GetName() const;
	virtual void	GetPosition(float&, float&) const = 0;

	virtual bool	InContent(float, float) const;
	virtual bool	InResize(float, float, float, int&) const;
	virtual bool	InRotate(float, float, float, int&) const;
	virtual bool	InTorsion(float, float, float, int&) const;

	virtual void	SetAngle(float, bool = false, bool = false) = 0;
	virtual void	SetName(const std::string&);
	virtual void	SetPosition(float, float, bool = false) = 0;
	virtual void	SetSize(float, float, float = 0, float = 0, float = 0, float = 0, bool = false, bool = false) = 0;
	virtual void	SetTorsion(float, float, bool = false) = 0;

	virtual void	ToAbsolute(float, float, float&, float&) const;
	virtual float	ToAngle(float, float) const;
	virtual void	ToRelative(float, float, float&, float&) const;

	virtual ErrID	FileLoad(FILE*);
	virtual ErrID	FileSave(FILE*) const;

	float			h;
	float			i;
	float			j;
	float			w;

public:
	virtual void	FinalizePosition(float, float);
	virtual void	FinalizeSize(float, float, float, float);

	const PsShape* parent;
	std::string		name;

	float			r;
	float			x;
	float			y;

	bool			hide;
	bool			constraint;
};
