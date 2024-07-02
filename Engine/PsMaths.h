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
#ifndef PS_MATHS_H__

#define PS_MATHS_H__

#include "PsTypes.h"

#define PS_MATH_PI 3.14159265358979323846f
#define PS_MATH_NEAR_ZERO 0.000001f

/*
** Un vecteur en pixel.
*/
struct PsVector
{
	float X;
	float Y;
	float Z;

	PsVector();

	PsVector(float, float, float);
	PsVector operator+(const PsVector&) const;
	PsVector& operator+=(const PsVector&);
	PsVector operator-(const PsVector&) const;
	PsVector& operator-=(const PsVector&);
	PsVector operator*(const float) const;
	PsVector operator/(const float) const;
	PsVector& operator/=(const float);

	float Size();
};

/*
** Une rotation en radians.
*/
struct PsRotator
{
	float Roll;
	float Pitch;
	float Yaw;

	PsRotator();

	static float ToDegree(float);
	static float FromDegree(float);
};

/**********************************************************************
** Fonctions variées.
*/

void TranslateVertex(float* A, float* V, float* R);
void RotateVertex(float* V, float* R, float fRoll, float fPitch, float fYaw);
void MultiplyVectorByMatrix(float* M, float* V, float* R);
bool GenerateInverseMatrix4f(float i[16], const float m[16]);
bool SameSign(float a, float b);

/**********************************************************************
** Fonctions variées appliquées aux PsVector
*/

float DotProduct2x2(PsVector& a, PsVector& b);
float DotProduct(PsVector& a, PsVector& b);

PsVector RotateVertex(PsVector& V, float fRoll, float fPitch, float fYaw);
PsVector* LinePlaneIntersection(PsVector&, PsVector&, PsVector&, PsVector&);

/**********************************************************************
** Fonctions spécfiques Windows.
*/
#ifdef _WINDOWS
double round(double);
#endif /* _WINDOWS */

/**********************************************************************
** Fonctions spécfiques Mac.
*/
#ifdef _MACOSX
float	min(float, float);
#endif /* _MACOSX */

#endif /* PS_MATHS_H__ */
