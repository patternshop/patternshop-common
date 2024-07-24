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
#include "PsShape.h"
#include "PsTexture.h"
#include "PsMessage.h"

class	PsMatrix;
class	PsRender;

/*
** Une image est une figure géometrique contenue dans une autre (une matrice). Voir la classe
** PsShape avant de lire celle-ci.
*/
class	PsImage : public PsShape
{
	friend class	PsRender;

public:
	PsImage(PsMatrix*);
	virtual			~PsImage();

	virtual PsMatrix* GetParent() const;
	virtual void	GetPosition(float&, float&) const;

	bool			TextureFromBuffer(uint8*, bool = true);
	bool			TextureFromFile(const char*, bool = true);

	virtual void	SetAngle(float, bool = false, bool = false);
	virtual void	SetPosition(float, float, bool = false);
	virtual void	SetSize(float, float, float = 0, float = 0, float = 0, float = 0, bool = false, bool = false);
	virtual void	SetTorsion(float, float, bool = false);

	PsTexture& GetTexture() { return texture; }

	const uint32	GetAutoGenId() { return texture.GetAutoGenId(); }


public:
	PsTexture		texture;
	float			corner[4][2];
};
