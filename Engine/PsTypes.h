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
#ifndef PS_TYPE_H__

#define PS_TYPE_H__

typedef unsigned char   uint8;
typedef unsigned int    uint32;


/*
** Safe casting
*/
int FloatToInt(const float);
int DoubleToInt(const double);

float CharToFloat(const char);
float IntToFloat(const int);
float DoubleToFloat(const double);

inline uint8 FloatToUint8(const float);

/*********************
** Windows
*/
#ifdef _WINDOWS

# include "stdafx.h"

typedef RECT PsRect;
typedef CPoint PsPoint;

#endif /* _WINDOWS */

/***********************
** Mac
*/
#ifdef _MACOSX

struct PsRect
{ 
   float  left, 
          right, 
          bottom, 
          top; 
};

struct PsPoint
{
	int	x,
			y;
};

char *strupr(char*);

#endif /* _MACOSX */

#endif /* PS_TYPE_H__ */
