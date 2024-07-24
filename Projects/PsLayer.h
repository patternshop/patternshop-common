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

#ifndef PS_LAYER_H__

#define PS_LAYER_H__

#include <stdio.h>
#include <list>

#include "PsShape.h"
#include "PsImage.h"
#include "PsMessage.h"
#include "PsMatrix.h"
#include "PsMaths.h"

class	PsProjectController;
class	PsRender;



/*
** Un layer est une couche de Patron.
*/
class	PsLayer
{
	friend class	PsProjectController;
	friend class	PsRender;

public:
	PsLayer();
	virtual			~PsLayer();

public:
	virtual ErrID	Register(int, int, uint8*);

public:
	void UpdateGizmoProjection(PsRender&);
	void UpdateProjection(PsRender&);
	void InitProjection(PsRender&);

public:
	bool MouseIsInside(int, int) const;
	bool InRotate(float, float, float, int&) const;
	float ToAngle(float, float) const;
	bool InResize(float, float, float, int&) const;

private:
	ErrID CreateTexture();

public: // persistant variables
	int iWidth, iHeight;
	uint8* ucData;

	PsVector vTranslation;
	PsRotator rRotation;
	float fScale;

	float fGizmoSize;

public: // temporary variables
	GLuint iMaskTextureId;
	GLuint iFinalTextureId;
	PsVector vProjectedGizmo[4];
	PsVector vProjected[4];
	float fW[4];

};

#endif /* PS_LAYER_H__ */
