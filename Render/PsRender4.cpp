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

/*
 ** Retrieve the repeated pattern over the entire document in a
 ** texture of size 'fTextureSize'.
 */
GLuint PsRender::CreateDocumentTexture(PsProjectController& project_controller)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, this->iLayerTextureSize, this->iLayerTextureSize);
	glOrtho(0, this->doc_x, this->doc_y, 0, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	this->DrawMatrices(project_controller);
	GLuint iTextureBloc;
	hardwareRenderer.CopyToHardBuffer(iTextureBloc, this->iLayerTextureSize, this->iLayerTextureSize);
	this->PrepareSurface(project_controller, this->size_x, this->size_y);
	return iTextureBloc;
}

/*
 ** Returns the eye position used in MultiLayerRendering mode (see PsPattern)
 ** to project_controller the different layers (see PsLayer).
 */
PsVector PsRender::GetEyeLocation()
{
	return PsVector(this->doc_x / 2.f, this->doc_y / 2.f, this->doc_y * 2.415f);
}

void PsRender::MultiLayerRendering(PsProjectController& project_controller, int x, int y)
{
	// pre-conditions
	if (!project_controller.pattern) return;

	//-- retrieve parameters
	PsPattern* pattern = project_controller.pattern;
	int iCount = pattern->GetChannelsCount();
	//--

	//iCount = 1; // FIXME

	this->PrepareSurface(project_controller, x, y);

	if (iCount > 0)
	{
		//-- HARDWARE mode
		if (this->engine == ENGINE_HARDWARE)
		{
#ifdef _MACOSX
			this->active->SetPixelBufferContext();
#endif

			GLuint iDocTexture = this->CreateDocumentTexture(project_controller);
			for (int i = 0; i < iCount; ++i)
			{
				this->UpdateLayerTexture(project_controller, pattern->aLayers[i], iDocTexture);
			}

#ifdef _MACOSX
			this->active->RestorePreviousContext();
#endif

			this->DrawBack(project_controller, (float)x, (float)y);
			for (int i = 0; i < iCount; ++i)
			{
				this->DrawLayerTexture(pattern->aLayers[i]->iFinalTextureId);
			}

			glDeleteTextures(1, &iDocTexture);

		}
		//-- otherwise SOFTWARE mode
		else
		{
			for (int i = 0; i < iCount; ++i)
			{
				DrawLayerSoftwareFile(i);
			}
		}
		//--
	}

	this->DrawPattern(*project_controller.pattern);
}

/**
 * Triggers a render, hardware or software, according to the chosen mode.
*/
void PsRender::Render(PsProjectController& project_controller, int x, int y)
{

	if (this->engine == ENGINE_HARDWARE)
	{
		glPushMatrix();
	}

	if (!project_controller.pattern
		|| project_controller.pattern->hide)
	{
		this->MonoLayerRendering(project_controller, x, y);
	}
	else
	{
		this->MultiLayerRendering(project_controller, x, y);
	}

	this->DrawImages(project_controller);

	if (this->engine == ENGINE_HARDWARE)
	{
		if (PsController::Instance().GetOption(PsController::OPTION_DOCUMENT_SHOW))
		{
			this->DrawDocument();
		}

		if (PsController::Instance().GetOption(PsController::OPTION_BOX_SHOW))
		{
			this->DrawGizmos(project_controller);
		}
		glPopMatrix();
	}
}

/*
 ** Change the document size.
 */
void PsRender::SetDocSize(int x, int y)
{
	this->doc_x = x;
	this->doc_y = y;

	if (PsController::Instance().project_controller && PsController::Instance().project_controller->pattern)
		PsController::Instance().project_controller->pattern->UpdateScale(this->doc_x, this->doc_y);
}

/*
 ** Change the rendering mode (software, hardware).
 */
void PsRender::SetEngine(Engine engine)
{
	this->engine = engine;
}

/*
 ** Change the scrolling values.
 */
void PsRender::SetScroll(float x, float y)
{
	this->scroll_x = x;
	this->scroll_y = y;

	this->Recalc();
}

/*
 ** Change the window size.
 */
void PsRender::SetSize(int x, int y)
{
	this->size_x = x;
	this->size_y = y;

	int iMax = this->size_x;
	if (this->size_y > iMax) iMax = this->size_y;

	this->fZoomMax = 1.5f / ((float)y / (float)this->doc_y);
	this->fZoomMin = 50.f / (float)iMax;

	this->Recalc();
}

/*
 ** Manually define a document area (used for software rendering).
 */
void PsRender::SetZone(float x, float y)
{
	this->x1 = 0;
	this->x2 = x;
	this->y1 = y;
	this->y2 = 0;
}

/*
 ** Change the zoom.
 */
void PsRender::SetZoom(float zoom)
{
	if (zoom < this->fZoomMin)
		zoom = this->fZoomMin;
	else if (zoom > this->fZoomMax)
		zoom = this->fZoomMax;

	this->zoom = zoom;

	this->Recalc();
}
