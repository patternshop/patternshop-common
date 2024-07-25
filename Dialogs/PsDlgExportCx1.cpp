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
PsProjectController* PsDlgExportCx::project_controller;
float PsDlgExportCx::r_backup;
int PsDlgExportCx::h;
int PsDlgExportCx::w;

bool PsDlgExportCx::Initialize()
{
	if (!PsController::Instance().project_controller)
		return false;

	//-- retrieving the project_controller
	this->project_controller = PsController::Instance().project_controller;
	assert(this->project_controller);
	assert(this->project_controller->matrix);
	assert(this->project_controller->matrix->i == 0);
	assert(this->project_controller->matrix->j == 0);
	this->dpi = this->project_controller->renderer.dpi;
	this->z = 100;
	//--

	if (!this->CheckRotation())
		if (!GetQuestion(QUESTION_EXPORT_ROTATION))
			return false;

	//-- retrieving the corners of the matrix
	this->TweakRotation();
	this->GetMatrixWindow();
	this->w = (int)(this->right - this->left);
	this->h = (int)(this->bottom - this->top);
	//--

	//-- rendering
	this->CreateExportImage();
	this->CreatePreviewImage();
	//--

	//-- restoring the rendering
	this->RestoreRotation();
	this->project_controller->renderer.Recalc();
	PsController::Instance().UpdateWindow();
	//--

	return true;
}
bool PsDlgExportCx::CheckRotation()
{
	int Angle = (int)round(project_controller->matrix->r * 180.f / PS_MATH_PI);
	if (project_controller->matrix->w != project_controller->matrix->h)
		if (Angle % 90 != 0)
			return false;
	if (project_controller->matrix->w == project_controller->matrix->h)
		if (Angle % 45 != 0)
			return false;
	return true;
}

/*
** Mise � plat de l'angle de la matrice
*/
void PsDlgExportCx::TweakRotation()
{
	r_backup = project_controller->matrix->r;
	if (!CheckRotation())
		project_controller->matrix->SetAngle(0, false, true);
}

/*
** R�tablisement de l'angle
*/
void PsDlgExportCx::RestoreRotation()
{
	project_controller->matrix->SetAngle(r_backup, false, true);
}

void PsDlgExportCx::GetMatrixWindow()
{
	project_controller->matrix->ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	project_controller->matrix->ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	project_controller->matrix->ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	project_controller->matrix->ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);
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
	project_controller = PsController::Instance().project_controller;
	SoftwareBuffer exportImage;

	if (!project_controller)
		return -1;

	if (!project_controller->matrix && project_controller->matrices.size() > 0)
		project_controller->matrix = *(project_controller->matrices.begin());

	if (!project_controller->matrix)
		return -1;

	PsRender& renderer = project_controller->renderer;

	//-- r�cup�ration des coins de la matrice
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
	renderer.DrawBack(*project_controller, exportImage.GetWidth(), exportImage.GetHeight());
	renderer.DrawShape(*(project_controller->matrix));
	hardwareRenderer.CopyToHardBuffer(texture, iWidth, iHeight);

	glPopMatrix();

	//-- r�tablisement du rendu
	RestoreRotation();
	project_controller->renderer.Recalc();
	PsController::Instance().UpdateWindow();
	//--

	return texture;
}

void PsDlgExportCx::CreateExportImage()
{
	//-- sizing the buffer
	double rw = (exportZone.width - 10) / (this->right - this->left);
	double rh = (exportZone.height - 20) / (this->bottom - this->top);
	double maxScale = rw;
	if (rh < rw) maxScale = rh;
	exportImage.Create((int)((this->right - this->left) * maxScale), (int)((this->bottom - this->top) * maxScale), 24);
	//--

	PsRender& renderer = this->project_controller->renderer;

	glPushMatrix();

	//-- centering the view on the matrix
	renderer.x1 = this->left;
	renderer.x2 = this->right;
	renderer.y1 = this->bottom;
	renderer.y2 = this->top;
	glViewport(0, 0, exportImage.GetWidth(), exportImage.GetHeight());
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(this->left, this->right, this->bottom, this->top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();
	//--

	//-- rendering into the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.DrawBack(*this->project_controller, exportImage.GetWidth(), exportImage.GetHeight());
	renderer.DrawShape(*(this->project_controller->matrix));
	hardwareRenderer.CopyToSoftBuffer(exportImage);

	glPopMatrix();
}

void PsDlgExportCx::CreatePreviewImage()
{
	//-- windowing
	this->GetMatrixWindow();
	this->right += (this->right - this->left) * 2;
	this->bottom += (this->bottom - this->top) * 2;
	//--

	//-- sizing the buffer
	double rw = (previewZone.width - 10) / (this->right - this->left) / 3;
	double rh = (previewZone.height - 20) / (this->bottom - this->top) / 3;
	double maxScale = rw;
	if (rh < rw) maxScale = rh;
	previewImage.Create((this->right - this->left) * maxScale * 3, (this->bottom - this->top) * maxScale * 3, 24);
	//--

	PsRender& renderer = this->project_controller->renderer;

	//-- start
	glPushMatrix();
	//--

	//-- centering the view on the matrix
	renderer.x1 = this->left;
	renderer.x2 = this->right;
	renderer.y1 = this->bottom;
	renderer.y2 = this->top;
	glViewport(0, 0, previewImage.GetWidth(), previewImage.GetHeight());
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(this->left, this->right, this->bottom, this->top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();
	//--

	//-- rendering into the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.DrawBack(*this->project_controller, previewImage.GetWidth(), previewImage.GetHeight());
	renderer.DrawShape(*(this->project_controller->matrix));
	hardwareRenderer.CopyToSoftBuffer(previewImage);
	//--

	glPopMatrix();
}

void PsDlgExportCx::OnValidation(const char* filename)
{
	PsController::Instance().SetProgress(-1);

	this->TweakRotation();

	// rounding the dimension in pixels
	double pixel_width = round(this->w * this->z / 100.f);
	double pixel_height = round(this->h * this->z / 100.f);
	//--

	//-- zooming the matrix
	float w_backup = this->project_controller->matrix->w;
	float h_backup = this->project_controller->matrix->h;
	this->project_controller->matrix->SetSize(this->project_controller->matrix->w * this->z / 100.f, this->project_controller->matrix->h * this->z / 100.f, 0, 0, 0, 0, false, true);
	//--

	//-- windowing
	this->GetMatrixWindow();
	//--

	PsRender& renderer = this->project_controller->renderer;

	//-- centering the view on the matrix
	renderer.x1 = this->left;
	renderer.x2 = this->right;
	renderer.y1 = this->bottom;
	renderer.y2 = this->top;
	//--

	PsController::Instance().SetProgress(5);

	//-- saving the dpi
	int dpi_backup = renderer.dpi;
	renderer.dpi = this->dpi;
	//--

	InitSoftwareFile(pixel_width, pixel_height);

	PsController::Instance().SetProgress(10);

	renderer.SetEngine(PsRender::ENGINE_SOFTWARE);
	renderer.DrawShape(*(this->project_controller->matrix));

	PsController::Instance().SetProgress(40);
	flushSoftwareFile(filename, this->project_controller->bHideColor);

	this->RestoreRotation();

	//-- restoring the original size
	this->project_controller->matrix->SetSize(w_backup, h_backup, 0, 0, 0, 0, false, true);
	//--

	//-- restoring the dpi
	renderer.dpi = dpi_backup;
	//--

	renderer.Recalc();

	PsController::Instance().SetProgress(90);
	PsController::Instance().SetProgress(-2);
}
