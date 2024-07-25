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

#include <list>
#include <math.h>
#include "PsRender.h"
#include "PsMessage.h"
#include "PsController.h"
#include "PsMatrix.h"

class	PsAction;
class	PsMatrix;

typedef std::list<PsAction*>	LogList;
typedef std::list<PsMatrix*>	MatrixList;

// Project parameters
#define ZOOM_COEF        0.02f
#define LOG_SIZE         50

class	PsProject
{

};
