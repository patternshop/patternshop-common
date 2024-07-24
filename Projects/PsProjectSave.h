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

#include "PsProject.h"

/*
** This class saves a project.
 */
class	PsProjectSave
{
	friend class PsProject;

public:
	PsProjectSave(PsProject& project);
	~PsProjectSave();

public:
	ErrID saveProject(const char* path);
	ErrID saveShape(PsShape& shape) const;
	ErrID saveImage(PsImage& image) const;
	ErrID saveMatrix(PsMatrix& matrix) const;
	ErrID saveLayer(PsLayer& layer) const;
	ErrID savePattern(PsPattern& pattern) const;

protected:
	PsProject& project;
	FILE* file;
};
