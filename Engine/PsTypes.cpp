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
#include "PsTypes.h"
#include <string.h>

#ifdef _WINDOWS

#endif /* _WINDOWS */

#ifdef _MACOSX
char* strupr(char* _s)
{
	char* rv = _s;
	while (*_s)
	{
		*_s = toupper((uint8)*_s);
		_s++;
	}
	return rv;
}

#endif /* _MACOSX */

/*
** Safe casting :
 ** FIXME : add assertions on bound exceptions
 */

int FloatToInt(const float n)
{
	return (int)n;
}

int DoubleToInt(const double n)
{
	return (int)n;
}

float IntToFloat(const int n)
{
	return (float)n;
}

float DoubleToFloat(const double n)
{
	return (float)n;
}

uint8 FloatToUint8(const float n)
{
	return (uint8)n;
}

float CharToFloat(const char c)
{
	return (float)c;
}