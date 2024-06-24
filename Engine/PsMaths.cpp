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
#include "PsMaths.h"
#include <string.h>
#include <math.h>

/*
** Multiplication d'un vecteur par une matrice.
** @param : M une matrice 3x3, V un vecteur 3x1 et R le résultat un vecteur 3x1.
*/
void MultiplyVectorByMatrix(float *M, float *V, float *R)
{
  for (int i = 0, k = 0; i < 3; ++i)
  {
    k = i * 3;
    R[i] = 0.f;
    for (int j = 0; j < 3; ++j)
      R[i] += M[k + j] * V[j];
  }
}

/*
** Rotation d'un point autour de l'origine.
** @param : V un point 3x1, R le résultat 3x1 et les angles en radian.
*/
void RotateVertex(float *V, float *R, float fRoll, float fPitch, float fYaw)
{
  float _Vs[3], _Vd[3];

  memcpy(_Vs, V, 3 * sizeof(float));

  if (fRoll != 0.f)
  {
    float Rx[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    Rx[0] = 1.f;
    Rx[4] = Rx[8] = cos(fRoll);
    Rx[5] = sin(fRoll);
    Rx[7] = -Rx[5];
    MultiplyVectorByMatrix(Rx, _Vs, _Vd);
    memcpy(_Vs, _Vd, 3 * sizeof(float));
  }

  if (fPitch != 0.f)
  {
    float Ry[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    Ry[4] = 1.f; 
    Ry[0] = Ry[8] = cos(fPitch);
    Ry[2] = sin(fPitch);
    Ry[6] = -Ry[2];
    MultiplyVectorByMatrix(Ry, _Vs, _Vd);
    memcpy(_Vs, _Vd, 3 * sizeof(float));
  }

  if (fYaw != 0.f)
  {
    float Rz[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    Rz[8] = 1.f;
    Rz[0] = Rz[4] = cos(fYaw);
    Rz[1] = sin(fYaw);
    Rz[3] = -Rz[1];
    MultiplyVectorByMatrix(Rz, _Vs, _Vd);
    memcpy(_Vs, _Vd, 3 * sizeof(float));
  }

  memcpy(R, _Vs, 3 * sizeof(float));

}

/*
** Rotation d'un point autour de l'origine.
** @param : V un vecteur 3x1 et les angles en radian.
*/
PsVector RotateVertex(PsVector &V, float fRoll, float fPitch, float fYaw)
{
  PsVector R;
  float _V[3], _R[3];

  _V[0] = V.X;
  _V[1] = V.Y;
  _V[2] = V.Z;

  RotateVertex(_V, _R, fRoll, fPitch, fYaw);
 
  R.X = _R[0];
  R.Y = _R[1];
  R.Z = _R[2];

  return R;
}

/*
** Translation d'un point par un vecteur.
** @param : A un point 3x1, V un vecteur 3x1, R le point résultat 3x1.
*/
void TranslateVertex(float *A, float *V, float *R)
{
  R[0] = A[0] + V[0];
  R[1] = A[1] + V[1];
  R[2] = A[2] + V[2];
}

PsVector::PsVector()
{
  X = 0.f;
  Y = 0.f;
  Z = 0.f;
}

PsVector::PsVector(float X,float Y,float Z)
{
  this->X = X;
  this->Y = Y;
  this->Z = Z;
}

PsVector PsVector::operator+(const PsVector &A) const
{
  PsVector R;
  R.X = X + A.X;
  R.Y = Y + A.Y;
  R.Z = Z + A.Z;
  return R;
}

PsVector &PsVector::operator+=(const PsVector &A)
{
  X = X + A.X;
  Y = Y + A.Y;
  Z = Z + A.Z;
  return *this;
}

PsVector PsVector::operator-(const PsVector &A) const
{
  PsVector R;
  R.X = X - A.X;
  R.Y = Y - A.Y;
  R.Z = Z - A.Z;
  return R;
}

PsVector &PsVector::operator-=(const PsVector &A)
{
  X = X - A.X;
  Y = Y - A.Y;
  Z = Z - A.Z;
  return *this;
}

PsVector PsVector::operator*(const float k) const
{
  PsVector R;
  R.X = X * k;
  R.Y = Y * k;
  R.Z = Z * k;
  return R;
}

PsVector PsVector::operator/(const float k) const
{
  PsVector R;
  R.X = X / k;
  R.Y = Y / k;
  R.Z = Z / k;
  return R;
}

PsVector &PsVector::operator/=(const float k)
{
  X = X / k;
  Y = Y / k;
  Z = Z / k;
  return *this;
}


/*
** Cette fonction calcule la norme d'un vecteur 3x1.
*/
float PsVector::Size()
{
  return sqrt(pow(X, 2.f) + pow(Y, 2.f) + pow(Z, 2.f));
}

PsRotator::PsRotator()
{
  Yaw = 0.f;
  Pitch = 0.f;
  Roll = 0.f;
}

float PsRotator::ToDegree(float r)
{ 
  return (r * 180.0f / PS_MATH_PI);
}

float PsRotator::FromDegree(float r)
{
  return (r * PS_MATH_PI / 180.0f);
}

#ifdef _WINDOWS
/*
** Cette fonction arrondie un double
*/
double round(double a)
{
  double ca = ceil(a);
  double fa = floor(a);

  if (ca - a < a - fa) return ca;
  else return fa;
}
#endif /* _WINDOWS */

#ifdef _MACOSX
/*
** Cette fonction retourne le plus petit des deux float
*/
float	min(float x, float y)
{
	return x <= y ? x : y;
}
#endif /* _MACOSX */

float Determinant4f(const float m[16])
{
  return
    m[12]*m[9]*m[6]*m[3]-
    m[8]*m[13]*m[6]*m[3]-
    m[12]*m[5]*m[10]*m[3]+
    m[4]*m[13]*m[10]*m[3]+
    m[8]*m[5]*m[14]*m[3]-
    m[4]*m[9]*m[14]*m[3]-
    m[12]*m[9]*m[2]*m[7]+
    m[8]*m[13]*m[2]*m[7]+
    m[12]*m[1]*m[10]*m[7]-
    m[0]*m[13]*m[10]*m[7]-
    m[8]*m[1]*m[14]*m[7]+
    m[0]*m[9]*m[14]*m[7]+
    m[12]*m[5]*m[2]*m[11]-
    m[4]*m[13]*m[2]*m[11]-
    m[12]*m[1]*m[6]*m[11]+
    m[0]*m[13]*m[6]*m[11]+
    m[4]*m[1]*m[14]*m[11]-
    m[0]*m[5]*m[14]*m[11]-
    m[8]*m[5]*m[2]*m[15]+
    m[4]*m[9]*m[2]*m[15]+
    m[8]*m[1]*m[6]*m[15]-
    m[0]*m[9]*m[6]*m[15]-
    m[4]*m[1]*m[10]*m[15]+
    m[0]*m[5]*m[10]*m[15];
}

/*
** Inversion d'une matrice 4x4
** i : out, m : in
*/
bool GenerateInverseMatrix4f(float i[16], const float m[16])
{
  float x=Determinant4f(m);
  if (x==0) return false;

  i[0]= (-m[13]*m[10]*m[7] +m[9]*m[14]*m[7] +m[13]*m[6]*m[11]
  -m[5]*m[14]*m[11] -m[9]*m[6]*m[15] +m[5]*m[10]*m[15])/x;
  i[4]= ( m[12]*m[10]*m[7] -m[8]*m[14]*m[7] -m[12]*m[6]*m[11]
  +m[4]*m[14]*m[11] +m[8]*m[6]*m[15] -m[4]*m[10]*m[15])/x;
  i[8]= (-m[12]*m[9]* m[7] +m[8]*m[13]*m[7] +m[12]*m[5]*m[11]
  -m[4]*m[13]*m[11] -m[8]*m[5]*m[15] +m[4]*m[9]* m[15])/x;
  i[12]=( m[12]*m[9]* m[6] -m[8]*m[13]*m[6] -m[12]*m[5]*m[10]
  +m[4]*m[13]*m[10] +m[8]*m[5]*m[14] -m[4]*m[9]* m[14])/x;
  i[1]= ( m[13]*m[10]*m[3] -m[9]*m[14]*m[3] -m[13]*m[2]*m[11]
  +m[1]*m[14]*m[11] +m[9]*m[2]*m[15] -m[1]*m[10]*m[15])/x;
  i[5]= (-m[12]*m[10]*m[3] +m[8]*m[14]*m[3] +m[12]*m[2]*m[11]
  -m[0]*m[14]*m[11] -m[8]*m[2]*m[15] +m[0]*m[10]*m[15])/x;
  i[9]= ( m[12]*m[9]* m[3] -m[8]*m[13]*m[3] -m[12]*m[1]*m[11]
  +m[0]*m[13]*m[11] +m[8]*m[1]*m[15] -m[0]*m[9]* m[15])/x;
  i[13]=(-m[12]*m[9]* m[2] +m[8]*m[13]*m[2] +m[12]*m[1]*m[10]
  -m[0]*m[13]*m[10] -m[8]*m[1]*m[14] +m[0]*m[9]* m[14])/x;
  i[2]= (-m[13]*m[6]* m[3] +m[5]*m[14]*m[3] +m[13]*m[2]*m[7]
  -m[1]*m[14]*m[7] -m[5]*m[2]*m[15] +m[1]*m[6]* m[15])/x;
  i[6]= ( m[12]*m[6]* m[3] -m[4]*m[14]*m[3] -m[12]*m[2]*m[7]
  +m[0]*m[14]*m[7] +m[4]*m[2]*m[15] -m[0]*m[6]* m[15])/x;
  i[10]=(-m[12]*m[5]* m[3] +m[4]*m[13]*m[3] +m[12]*m[1]*m[7]
  -m[0]*m[13]*m[7] -m[4]*m[1]*m[15] +m[0]*m[5]* m[15])/x;
  i[14]=( m[12]*m[5]* m[2] -m[4]*m[13]*m[2] -m[12]*m[1]*m[6]
  +m[0]*m[13]*m[6] +m[4]*m[1]*m[14] -m[0]*m[5]* m[14])/x;
  i[3]= ( m[9]* m[6]* m[3] -m[5]*m[10]*m[3] -m[9]* m[2]*m[7]
  +m[1]*m[10]*m[7] +m[5]*m[2]*m[11] -m[1]*m[6]* m[11])/x;
  i[7]= (-m[8]* m[6]* m[3] +m[4]*m[10]*m[3] +m[8]* m[2]*m[7]
  -m[0]*m[10]*m[7] -m[4]*m[2]*m[11] +m[0]*m[6]* m[11])/x;
  i[11]=( m[8]* m[5]* m[3] -m[4]*m[9]* m[3] -m[8]* m[1]*m[7]
  +m[0]*m[9]* m[7] +m[4]*m[1]*m[11] -m[0]*m[5]* m[11])/x;
  i[15]=(-m[8]* m[5]* m[2] +m[4]*m[9]* m[2] +m[8]* m[1]*m[6]
  -m[0]*m[9]* m[6] -m[4]*m[1]*m[10] +m[0]*m[5]* m[10])/x;

  return true;
} 

bool SameSign(float a, float b)
{
  if (a >= 0 && b >= 0)
    return true;

  if (a < 0 && b < 0)
    return true;
  
  return false;
}

float DotProduct2x2(PsVector &a, PsVector &b)
{
  return a.X * b.X + a.Y * b.Y; 
} 

float DotProduct(PsVector &a, PsVector &b)
{
  return a.X * b.X + a.Y * b.Y + a.Z * b.Z; 
} 

/*
** Cette fonction calcule le point d'intersection entre une droite et un plan
** @return : NULL si il n'y a pas d'intersection
*/
PsVector *LinePlaneIntersection(PsVector &linePoint, PsVector &lineVect, PsVector &planePoint, PsVector &planeNormal)
{
  float d = DotProduct(lineVect, planeNormal);
  if (-PS_MATH_NEAR_ZERO < d && d < PS_MATH_NEAR_ZERO)
    return NULL;
  PsVector v = linePoint - planePoint;
  float t = DotProduct(v, planeNormal) / d;
  PsVector p = linePoint + lineVect * -t;
  return new PsVector(p);
}
