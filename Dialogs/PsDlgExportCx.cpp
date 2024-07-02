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

void Get_WRITE_FORMATS(char*);

double PsDlgExportCx::left;
double PsDlgExportCx::right;
double PsDlgExportCx::bottom;
double PsDlgExportCx::top;
float PsDlgExportCx::corner[4][2];
PsProject* PsDlgExportCx::project;
float PsDlgExportCx::r_backup;
int PsDlgExportCx::h;
int PsDlgExportCx::w;

bool PsDlgExportCx::Initialize()
{
	if (!PsController::Instance().project)
		return false;

	//-- récupération du projet
	project = PsController::Instance().project;
	assert(project);
	assert(project->matrix);
	assert(project->matrix->i == 0);
	assert(project->matrix->j == 0);
	dpi = project->renderer.dpi;
	z = 100;
	//--

	if (!CheckRotation())
		if (!GetQuestion(QUESTION_EXPORT_ROTATION))
			return false;


	//-- récupération des coins de la matrice
	TweakRotation();
	GetMatrixWindow();
	w = (int)(right - left);
	h = (int)(bottom - top);
	//--

	//-- rendus
	CreateExportImage();
	CreatePreviewImage();
	//--

	//-- rétablisement du rendu
	RestoreRotation();
	project->renderer.Recalc();
	PsController::Instance().UpdateWindow();
	//--

	return true;
}

bool PsDlgExportCx::CheckRotation()
{
	int Angle = (int)round(project->matrix->r * 180.f / PS_MATH_PI);
	if (project->matrix->w != project->matrix->h)
		if (Angle % 90 != 0)
			return false;
	if (project->matrix->w == project->matrix->h)
		if (Angle % 45 != 0)
			return false;
	return true;
}

/*
** Mise à plat de l'angle de la matrice
*/
void PsDlgExportCx::TweakRotation()
{
	r_backup = project->matrix->r;
	if (!CheckRotation())
		project->matrix->SetAngle(0, false, true);
}

/*
** Rétablisement de l'angle
*/
void PsDlgExportCx::RestoreRotation()
{
	project->matrix->SetAngle(r_backup, false, true);
}

void PsDlgExportCx::GetMatrixWindow()
{
	project->matrix->ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	project->matrix->ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	project->matrix->ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	project->matrix->ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);
	left = corner[0][0];
	for (int i = 0; i < 4; ++i)
		if (corner[i][0] < left)
			left = corner[i][0];
	right = corner[0][0];
	for (int i = 0; i < 4; ++i)
		if (corner[i][0] > right)
			right = corner[i][0];
	bottom = corner[0][1];
	for (int i = 0; i < 4; ++i)
		if (corner[i][1] > bottom)
			bottom = corner[i][1];
	top = corner[0][1];
	for (int i = 0; i < 4; ++i)
		if (corner[i][1] < top)
			top = corner[i][1];
}

GLuint PsDlgExportCx::CreateExportTexture(int iMaxSize)
{
	project = PsController::Instance().project;
	SoftwareBuffer exportImage;

	if (!project)
		return -1;

	if (!project->matrix && project->matrices.size() > 0)
		project->matrix = *(project->matrices.begin());

	if (!project->matrix)
		return -1;

	PsRender& renderer = project->renderer;

	//-- récupération des coins de la matrice
	TweakRotation();
	GetMatrixWindow();
	w = (int)(right - left);
	h = (int)(bottom - top);
	//--

	//-- dimensionnement du buffer
	double rw = iMaxSize / (right - left);
	double rh = iMaxSize / (bottom - top);
	double maxScale = rw;
	if (rh < rw) maxScale = rh;
	int iWidth = (int)((right - left) * maxScale);
	int iHeight = (int)((bottom - top) * maxScale);
	exportImage.Create(iWidth, iHeight, 24);
	//--

	glPushMatrix();

	//-- centrage de la vue sur la matrice
	renderer.x1 = left;
	renderer.x2 = right;
	renderer.y1 = bottom;
	renderer.y2 = top;
	glViewport(0, 0, exportImage.GetWidth(), exportImage.GetHeight());
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();
	//--

	//-- rendu dans la texture
	GLuint texture;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.DrawBack(*project, exportImage.GetWidth(), exportImage.GetHeight());
	renderer.DrawShape(*(project->matrix));
	hardwareRenderer.CopyToHardBuffer(texture, iWidth, iHeight);

	glPopMatrix();

	//-- rétablisement du rendu
	RestoreRotation();
	project->renderer.Recalc();
	PsController::Instance().UpdateWindow();
	//--

	return texture;
}

void PsDlgExportCx::CreateExportImage()
{

	//-- dimensionnement du buffer
	double rw = (exportZone.width - 10) / (right - left);
	double rh = (exportZone.height - 20) / (bottom - top);
	double maxScale = rw;
	if (rh < rw) maxScale = rh;
	exportImage.Create((int)((right - left) * maxScale), (int)((bottom - top) * maxScale), 24);
	//--

	PsRender& renderer = project->renderer;

	glPushMatrix();

	//-- centrage de la vue sur la matrice
	renderer.x1 = left;
	renderer.x2 = right;
	renderer.y1 = bottom;
	renderer.y2 = top;
	glViewport(0, 0, exportImage.GetWidth(), exportImage.GetHeight());
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();
	//--

	//-- rendu dans le backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.DrawBack(*project, exportImage.GetWidth(), exportImage.GetHeight());
	renderer.DrawShape(*(project->matrix));
	hardwareRenderer.CopyToSoftBuffer(exportImage);

	glPopMatrix();
}


void PsDlgExportCx::CreatePreviewImage()
{

	//-- fenêtrage
	GetMatrixWindow();
	right += (right - left) * 2;
	bottom += (bottom - top) * 2;
	//--

	//-- dimensionnement du buffer
	double rw = (previewZone.width - 10) / (right - left) / 3;
	double rh = (previewZone.height - 20) / (bottom - top) / 3;
	double maxScale = rw;
	if (rh < rw) maxScale = rh;
	previewImage.Create((right - left) * maxScale * 3, (bottom - top) * maxScale * 3, 24);
	//--

	PsRender& renderer = project->renderer;

	//-- début

	glPushMatrix();
	//--

	//-- centrage de la vue sur la matrice
	renderer.x1 = left;
	renderer.x2 = right;
	renderer.y1 = bottom;
	renderer.y2 = top;
	glViewport(0, 0, previewImage.GetWidth(), previewImage.GetHeight());
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();
	//--

	//-- rendu dans le backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.DrawBack(*project, previewImage.GetWidth(), previewImage.GetHeight());
	renderer.DrawShape(*(project->matrix));
	hardwareRenderer.CopyToSoftBuffer(previewImage);
	//--

	glPopMatrix();
}

void PsDlgExportCx::OnValidation(const char* filename)
{
	PsController::Instance().SetProgress(-1);

	TweakRotation();

	// arrondi de la dimension en pixels
	double pixel_width = round(w * z / 100.f);
	double pixel_height = round(h * z / 100.f);
	//--

	//-- zoom de la matrice
	float w_backup = project->matrix->w;
	float h_backup = project->matrix->h;
	project->matrix->SetSize(project->matrix->w * z / 100.f, project->matrix->h * z / 100.f, 0, 0, 0, 0, false, true);
	//--

	//-- fenêtrage
	GetMatrixWindow();
	//--

	PsRender& renderer = project->renderer;

	//-- centrage de la vue sur la matrice
	renderer.x1 = left;
	renderer.x2 = right;
	renderer.y1 = bottom;
	renderer.y2 = top;
	//--

	PsController::Instance().SetProgress(5);

	//-- enregistrement du dpi
	int dpi_backup = renderer.dpi;
	renderer.dpi = dpi;
	//--

	InitSoftwareFile(pixel_width, pixel_height);

	PsController::Instance().SetProgress(10);

	renderer.SetEngine(PsRender::ENGINE_SOFTWARE);
	renderer.DrawShape(*(project->matrix));

	PsController::Instance().SetProgress(40);
	flushSoftwareFile(filename, project->bHideColor);


	RestoreRotation();

	//-- restaurantion de la taille originale
	project->matrix->SetSize(w_backup, h_backup, 0, 0, 0, 0, false, true);
	//--

	//-- restaurantion du dpi
	renderer.dpi = dpi_backup;
	//--

	renderer.Recalc();

	PsController::Instance().SetProgress(90);
	PsController::Instance().SetProgress(-2);
}

void PsDlgExportCx::SetZ(double z_)
{
	z = z_;
	Update();
}

void PsDlgExportCx::GetTextValue(double px, int indexType, char* buffer)
{
	double v = -1;
	switch (indexType)
	{
	case 0:
		v = px / dpi;
		sprintf(buffer, "%.3f", v);
		break;
	case 1:
		v = px / (dpi / 2.54f);
		sprintf(buffer, "%.2f", v);
		break;
	case 2:
		v = px * 10.f / (dpi / 2.54f);
		sprintf(buffer, "%.3f", v);
		break;
	case 3:
		v = px / dpi / 0.0138888f;
		sprintf(buffer, "%.1f", v);
		break;
	case 4:
		v = px / dpi * 6.f;
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
	case 0: return v * dpi; break;
	case 1: return v * dpi / 2.54f; break;
	case 2: return v * (dpi / 2.54f) / 10.f; break;
	case 3: return v * dpi * 0.0138888f; break;
	case 4: return v * dpi / 6.f; break;
	case 5: return v; break;
	default: break;
	}
	return -1;
}
