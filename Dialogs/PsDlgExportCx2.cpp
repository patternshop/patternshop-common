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
#include "PsDlgExportCx.h"
#include "PsController.h"
#include "PsSoftRender.h"
#include "PsMaths.h"

#include <assert.h>

void PsDlgExportCx::SetZ(double z_)
{
	this->z = z_;
	this->Update();
}

void PsDlgExportCx::GetTextValue(double px, int indexType, char* buffer)
{
	double v = -1;
	switch (indexType)
	{
	case 0:
		v = px / this->dpi;
		sprintf(buffer, "%.3f", v);
		break;
	case 1:
		v = px / (this->dpi / 2.54f);
		sprintf(buffer, "%.2f", v);
		break;
	case 2:
		v = px * 10.f / (this->dpi / 2.54f);
		sprintf(buffer, "%.3f", v);
		break;
	case 3:
		v = px / this->dpi / 0.0138888f;
		sprintf(buffer, "%.1f", v);
		break;
	case 4:
		v = px / this->dpi * 6.f;
		sprintf(buffer, "%.1f", v);
		break;
	case 5:
		v = px;
		sprintf(buffer, "%.0f", v);
		break;
	default:
		sprintf(buffer, "?");
		break;
	}
}

double PsDlgExportCx::GetDoubleValue(int indexType, char* buffer)
{
	double v = atof(buffer);
	switch (indexType)
	{
	case 0: return v * this->dpi; break;
	case 1: return v * this->dpi / 2.54f; break;
	case 2: return v * (this->dpi / 2.54f) / 10.f; break;
	case 3: return v * this->dpi * 0.0138888f; break;
	case 4: return v * this->dpi / 6.f; break;
	case 5: return v; break;
	default: break;
	}
	return -1;
}
