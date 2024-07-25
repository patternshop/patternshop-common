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
#ifdef _WINDOWS
# include "stdafx.h"
# include <atlimage.h>
# include "Patternshop.h"
#endif

#ifdef _MACOSX
# include "PsMacView.h"
#endif

#include "PsRender.h"
#include "PsController.h"
#include "PsTexture.h"
#include "PsProjectController.h"
#include "PsSoftRender.h"
#include "PsHardware.h"
#include "PsMaths.h"
#include "PsTypes.h"

void PsRender::DrawStandardGizmos(PsProjectController& project_controller)
{
	this->DrawMatricesGizmos(project_controller);

	if (project_controller.image && !project_controller.image->hide && !project_controller.matrix)
	{
		glColor4f(1.f, 1.f, 1.f, 0.8f);
		this->DrawShape(*project_controller.image);
	}

	if (project_controller.image && !project_controller.image->hide &&
		(!project_controller.matrix || !project_controller.matrix->hide))
	{
		this->DrawBox(*project_controller.image);
	}

	if (project_controller.matrix && !project_controller.matrix->hide)
		this->DrawBoxHandle(*project_controller.matrix);
}

void PsRender::DrawGizmos(PsProjectController& project_controller)
{
	if (project_controller.bPatternsIsSelected && project_controller.pattern && !project_controller.pattern->hide)
	{
		this->DrawPatternGizmo(*project_controller.pattern, project_controller.iLayerId);
	}
	else
	{
		this->DrawStandardGizmos(project_controller);
	}
}

void PsRender::MonoLayerRendering(PsProjectController& project_controller, int x, int y)
{
	this->PrepareSurface(project_controller, x, y);

	if (this->engine == ENGINE_HARDWARE)
		this->DrawBack(project_controller, (float)x, (float)y);

	this->DrawMatrices(project_controller);

	if (project_controller.pattern && !project_controller.pattern->hide)
		this->DrawPattern(*project_controller.pattern/*, true*/);
}

void PsRender::DrawLayerTexture(GLuint layer)
{
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, layer);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, this->doc_y);
	glTexCoord2f(1, 0);
	glVertex2f(this->doc_x, this->doc_y);
	glTexCoord2f(1, 1);
	glVertex2f(this->doc_x, 0);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void PsRender::UpdateLayerTexture(PsProjectController& project_controller, PsLayer* layer, GLuint iDocTexture)
{
	PsPattern* pattern = project_controller.pattern;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, this->iLayerTextureSize, this->iLayerTextureSize);
	glOrtho(0, this->doc_x, this->doc_y, 0, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//-- display the plane
	float fDup = 9.f;
	float fScale = layer->fScale;
	layer->fScale *= fDup;
	layer->UpdateProjection(*this);
	layer->fScale = fScale;
	PsVector* vCorners = layer->vProjected;
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, iDocTexture);
	glBegin(GL_QUADS);
	glTexCoord4f(0.f * layer->fW[0], fDup * layer->fW[0], 0.f, layer->fW[0]);
	glVertex2f(vCorners[0].X, vCorners[0].Y);
	glTexCoord4f(fDup * layer->fW[1], fDup * layer->fW[1], 0.f, layer->fW[1]);
	glVertex2f(vCorners[1].X, vCorners[1].Y);
	glTexCoord4f(fDup * layer->fW[2], 0.f, 0.f, layer->fW[2]);
	glVertex2f(vCorners[2].X, vCorners[2].Y);
	glTexCoord4f(0.f, 0.f, 0.f, layer->fW[3]);
	glVertex2f(vCorners[3].X, vCorners[3].Y);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	layer->UpdateProjection(*this);
	//--

	//-- display the mask
	if (this->engine == ENGINE_HARDWARE)
	{
		glColor4f(1.f, 1.f, 1.f, 1.f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, layer->iMaskTextureId);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, this->doc_y);
		glTexCoord2f(1, 1);
		glVertex2f(this->doc_x, this->doc_y);
		glTexCoord2f(1, 0);
		glVertex2f(this->doc_x, 0);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	//--

	hardwareRenderer.CopyToHardBuffer(layer->iFinalTextureId, this->iLayerTextureSize, this->iLayerTextureSize, layer->iFinalTextureId == NULL);
	this->PrepareSurface(project_controller, this->size_x, this->size_y);
}
