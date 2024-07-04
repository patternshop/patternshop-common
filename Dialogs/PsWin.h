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
#ifndef PS_WIN_H__

#define PS_WIN_H__

#include "PsHardware.h"

class PsWin
{
public:
	typedef enum
	{
		PSFONT_NORMAL,
		PSFONT_BOLD
	} PsFont;

public:
	virtual void SetDashBlackPen() = 0;
	virtual void SetPenColor(int, int, int) = 0;
	virtual void SetBrushColor(int, int, int) = 0;
	virtual void SetTextColor(int, int, int) = 0;
	virtual void DrawRectangle(int, int, int, int) = 0;
	virtual void MovePenTo(int, int) = 0;
	virtual void DrawLineTo(int, int) = 0;
	virtual void DrawSoftwareBuffer(SoftwareBuffer&, int, int) = 0;
	virtual void DrawText(const char*, PsRect&, PsFont) = 0;
	virtual void DrawPolygon(PsPoint*, int) = 0;
	virtual void SetTarget(SoftwareBuffer*) = 0;

public:
	uint32 iWidth, iHeight;
};

#endif /* PS_WIN_H__ */

