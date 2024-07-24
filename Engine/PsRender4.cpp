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
#include "PsProject.h"
#include "PsSoftRender.h"
#include "PsHardware.h"
#include "PsMaths.h"
#include "PsTypes.h"

void PsRender::DrawStandardGizmos(PsProject& project)
{
	DrawMatricesGizmos(project);

	if (project.image && !project.image->hide && !project.matrix)
	{
		glColor4f(1.f, 1.f, 1.f, 0.8f);
		DrawShape(*project.image);
	}

	if (project.image && !project.image->hide &&
		(!project.matrix || !project.matrix->hide))
	{
		DrawBox(*project.image);
	}

	if (project.matrix && !project.matrix->hide)
		DrawBoxHandle(*project.matrix);
}

void PsRender::DrawGizmos(PsProject& project)
{
	if (project.bPatternsIsSelected && project.pattern && !project.pattern->hide)
	{
		DrawPatternGizmo(*project.pattern, project.iLayerId);
	}
	else
	{
		DrawStandardGizmos(project);
	}
}

void PsRender::MonoLayerRendering(PsProject& project, int x, int y)
{
	PrepareSurface(project, x, y);

	if (engine == ENGINE_HARDWARE)
		DrawBack(project, (float)x, (float)y);

	DrawMatrices(project);

	if (project.pattern && !project.pattern->hide)
		DrawPattern(*project.pattern/*, true*/);
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
	glVertex2f(0, doc_y);
	glTexCoord2f(1, 0);
	glVertex2f(doc_x, doc_y);
	glTexCoord2f(1, 1);
	glVertex2f(doc_x, 0);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}



void PsRender::UpdateLayerTexture(PsProject& project, PsLayer* layer, GLuint iDocTexture)
{
	PsPattern* pattern = project.pattern;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, iLayerTextureSize, iLayerTextureSize);
	glOrtho(0, doc_x, doc_y, 0, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//-- affichage du plan
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

	//-- affichage du masque
	if (engine == ENGINE_HARDWARE)
	{
		glColor4f(1.f, 1.f, 1.f, 1.f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, layer->iMaskTextureId);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, doc_y);
		glTexCoord2f(1, 1);
		glVertex2f(doc_x, doc_y);
		glTexCoord2f(1, 0);
		glVertex2f(doc_x, 0);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	//--

	hardwareRenderer.CopyToHardBuffer(layer->iFinalTextureId, iLayerTextureSize, iLayerTextureSize, layer->iFinalTextureId == NULL);
	PrepareSurface(project, size_x, size_y);
}

/*
 ** Récupération du motif répété sur tout le document dans une
 ** texture de taille 'fTextureSize'.
 */
GLuint PsRender::CreateDocumentTexture(PsProject& project)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, iLayerTextureSize, iLayerTextureSize);
	glOrtho(0, doc_x, doc_y, 0, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawMatrices(project);
	GLuint iTextureBloc;
	hardwareRenderer.CopyToHardBuffer(iTextureBloc, iLayerTextureSize, iLayerTextureSize);
	PrepareSurface(project, size_x, size_y);
	return iTextureBloc;
}

/*
 ** Renvoie la position de l'oeil utilisée en mode MultiLayerRendering (voir PsPattern)
 ** pour projeter les différentes couches (voir PsLayer).
 */
PsVector PsRender::GetEyeLocation()
{
	return PsVector(doc_x / 2.f, doc_y / 2.f, doc_y * 2.415f);
}

void PsRender::MultiLayerRendering(PsProject& project, int x, int y)
{
	// pré-conditions
	if (!project.pattern) return;

	//-- récupération des paramètres
	PsPattern* pattern = project.pattern;
	int iCount = pattern->GetChannelsCount();
	//--

	//iCount = 1; // FIXME

	this->PrepareSurface(project, x, y);

	if (iCount > 0)
	{
		//-- mode HARDWARE
		if (engine == ENGINE_HARDWARE)
		{
#ifdef _MACOSX
			active->SetPixelBufferContext();
#endif

			GLuint iDocTexture = this->CreateDocumentTexture(project);
			for (int i = 0; i < iCount; ++i)
			{
				this->UpdateLayerTexture(project, pattern->aLayers[i], iDocTexture);
			}

#ifdef _MACOSX
			active->RestorePreviousContext();
#endif

			this->DrawBack(project, (float)x, (float)y);
			for (int i = 0; i < iCount; ++i)
			{
				this->DrawLayerTexture(pattern->aLayers[i]->iFinalTextureId);
			}

			glDeleteTextures(1, &iDocTexture);

		}
		//-- sinon mode SOFTWARE
		else
		{
			for (int i = 0; i < iCount; ++i)
			{
				DrawLayerSoftwareFile(i);
			}
		}
		//--
	}

	this->DrawPattern(*project.pattern);
}

/**
 * Déclanche un rendu, hardware ou software, selon le mode choisi.
*/
void PsRender::Render(PsProject& project, int x, int y)
{

	if (engine == ENGINE_HARDWARE)
	{
		glPushMatrix();
	}

	if (!project.pattern
		|| project.pattern->hide)
	{
		this->MonoLayerRendering(project, x, y);
	}
	else
	{
		this->MultiLayerRendering(project, x, y);
	}

	this->DrawImages(project);

	if (engine == ENGINE_HARDWARE)
	{
		if (PsController::Instance().GetOption(PsController::OPTION_DOCUMENT_SHOW))
		{
			this->DrawDocument();
		}

		if (PsController::Instance().GetOption(PsController::OPTION_BOX_SHOW))
		{
			this->DrawGizmos(project);
		}
		glPopMatrix();
	}
}

/*
 ** Change la taille du document.
 */
void	PsRender::SetDocSize(int x, int y)
{
	doc_x = x;
	doc_y = y;

	if (PsController::Instance().project && PsController::Instance().project->pattern)
		PsController::Instance().project->pattern->UpdateScale(doc_x, doc_y);
}

/*
 ** Change le mode de rendu(software, hardware).
 */
void	PsRender::SetEngine(Engine engine)
{
	this->engine = engine;
}

/*
 ** Change les valeurs de scrolling.
 */
void	PsRender::SetScroll(float x, float y)
{
	scroll_x = x;
	scroll_y = y;

	Recalc();
}

/*
 ** Change la taille de la fenêtre.
 */
void	PsRender::SetSize(int x, int y)
{
	size_x = x;
	size_y = y;

	int iMax = size_x;
	if (size_y > iMax) iMax = size_y;

	fZoomMax = 1.5f / ((float)y / (float)doc_y);
	fZoomMin = 50.f / (float)iMax;

	Recalc();
}

/*
 ** Définit manuellement une zone de document(utilisé pour le rendu software).
 */
void PsRender::SetZone(float x, float y)
{
	x1 = 0;
	x2 = x;
	y1 = y;
	y2 = 0;
}

/*
 ** Change le zoom.
 */
void PsRender::SetZoom(float zoom)
{
	if (zoom < fZoomMin)
		zoom = fZoomMin;
	else if (zoom > fZoomMax)
		zoom = fZoomMax;

	this->zoom = zoom;

	Recalc();
}
