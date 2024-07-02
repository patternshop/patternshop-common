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
#ifndef DIALOG_PATTERN_CORE_H__

#define DIALOG_PATTERN_CORE_H__

#include "PsHardware.h"

class DialogPatternCx
{

public:
	DialogPatternCx();

public:
	void Update();
	void SetQuality(int);
	void UpdateMiniImage(int, int);

public:
	void OnShowWindow(bool);
	void OnButtonDown(int, int);

public:
	SoftwareBuffer m_RenduImage;
	int dst_x1, dst_y1;
	int dst_x2, dst_y2;
	int dst_width, dst_height;
	bool bUpdated;
};


#endif /* DIALOG_PATTERN_CORE_H__ */
