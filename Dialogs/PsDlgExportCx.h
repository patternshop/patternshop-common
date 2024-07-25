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
#ifndef PS_DLG_EXPORT_CX__

#define PS_DLG_EXPORT_CX__

#include "PsHardware.h"
#include "PsProjectController.h"

class PsDlgExportCx
{
protected:
	bool Initialize();
	static bool CheckRotation();
	static void TweakRotation();
	static void RestoreRotation();
	static void GetMatrixWindow();
	void OnValidation(const char*);
	void SetZ(double);
	void GetTextValue(double, int, char*);
	double GetDoubleValue(int, char*);

public:
	virtual void Update() = 0;
	static GLuint CreateExportTexture(int);
	void CreateExportImage();
	void CreatePreviewImage();

public:
	double dpi;
	static int h, w;
	double z;

protected:
	static PsProjectController* project_controller;
	static float r_backup;
	static double left, right, bottom, top;
	static float	corner[4][2];

	SoftwareBuffer exportImage;
	SoftwareBuffer previewImage;

	struct MyZone
	{
		int x, y; // coin supérieur gauche
		int width, height; // largeur, hauteur
	};

	MyZone exportZone;
	MyZone previewZone;
};

#endif /* PS_DLG_EXPORT_CX__ */

