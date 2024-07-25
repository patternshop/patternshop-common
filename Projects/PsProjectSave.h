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

#include "PsProjectController.h"

// File format version
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define CANDIDATE_VERSION 2

#define PROJECT_VERSION_OLD_BETA 3213654 // 111222333

//--------------------------AUTO--------------------------

#define PROJECT_VERSION    (MAJOR_VERSION * 1000000 + MINOR_VERSION * 1000 + CANDIDATE_VERSION)

#define BIG_ENDIAN_FILE 1
#define LITTLE_ENDIAN_FILE 0

#ifdef __BIG_ENDIAN__
#define ACCEPTED_ENDIAN_FILE BIG_ENDIAN_FILE
#else
#define ACCEPTED_ENDIAN_FILE LITTLE_ENDIAN_FILE
#endif

//--------------------------AUTO (END)----------------------

/*
** This class saves a project_controller.
 */
class	PsProjectSave
{
	friend class PsProjectController;

public:
	PsProjectSave(PsProjectController& project_controller);
	~PsProjectSave();

public:
	ErrID saveProject(const char* path);
	ErrID saveShape(PsShape& shape) const;
	ErrID saveImage(PsImage& image) const;
	ErrID saveMatrix(PsMatrix& matrix) const;
	ErrID saveLayer(PsLayer& layer) const;
	ErrID savePattern(PsPattern& pattern) const;

protected:
	PsProjectController& project_controller;
	FILE* file;
};
