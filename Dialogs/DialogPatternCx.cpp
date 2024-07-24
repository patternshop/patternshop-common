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
#include "PsProjectController.h"
#include "DialogPatternCx.h"

DialogPatternCx::DialogPatternCx()
{
	this->bUpdated = false;
	this->m_RenduImage.Create(1, 1, 24);
}

void DialogPatternCx::Update()
{
	this->bUpdated = false;
}

void DialogPatternCx::SetQuality(int iQuality)
{
	if (!PsController::Instance().project) return;
	PsProjectController& project = *(PsController::Instance().project);
	if (!project.pattern) return;
	project.renderer.iLayerTextureSize = FloatToInt(pow(2.f, 8.f + (float)iQuality));
	for (int i = 0; i < project.pattern->GetChannelsCount(); ++i)
	{
		PsLayer* layer = project.pattern->aLayers[i];
		if (layer->iFinalTextureId)
		{
			glDeleteTextures(1, &layer->iFinalTextureId);
			layer->iFinalTextureId = 0;
		}
	}
	PsController::Instance().UpdateWindow();
}

void DialogPatternCx::OnShowWindow(bool bShow)
{
	if (!bShow || !PsController::Instance().project || !PsController::Instance().project->pattern)
	{
		if (PsController::Instance().project)
		{
			if (PsController::Instance().project->bPatternsIsSelected)
			{
				PsController::Instance().project->bPatternsIsSelected = false;
				PsController::Instance().UpdateWindow();
			}
		}
	}
	else
	{
		if (PsController::Instance().project)
		{
			if (!PsController::Instance().project->bPatternsIsSelected)
			{
				PsController::Instance().project->bPatternsIsSelected = true;
				PsController::Instance().UpdateWindow();
			}
		}
		this->Update();
	}
}

void DialogPatternCx::OnButtonDown(int iX, int iY)
{
	if (!PsController::Instance().project || !PsController::Instance().project->pattern)
	{
		this->OnShowWindow(false);
		return;
	}

	PsProjectController& project = *(PsController::Instance().project);
	if (iX > this->dst_x1 && iX < this->dst_x2 && iY < this->dst_y2 && iY > this->dst_y1)
	{
		float r1 = (float)project.pattern->GetWidth() / (float)(this->dst_x2 - this->dst_x1);
		float r2 = (float)project.pattern->GetHeight() / (float)(this->dst_y2 - this->dst_y1);
		int iTargetSelected = -1;
		uint8 cMax = 150;
		int x = FloatToInt((iX - this->dst_x1) * r1);
		int y = FloatToInt((iY - this->dst_y1) * r2);
		int p = x + y * project.pattern->texture.width;
		for (int i = 0; i < project.pattern->GetChannelsCount(); ++i)
		{
			uint32 c = project.pattern->aLayers[i]->ucData[p];
			if (c >= cMax)
			{
				cMax = c;
				iTargetSelected = i;
			}
		}
		if (iTargetSelected != -1)
		{
			project.bPatternsIsSelected = true;
			project.iLayerId = iTargetSelected;
			PsController::Instance().UpdateWindow();
			this->Update();
		}
	}
}

void DialogPatternCx::UpdateMiniImage(int iWindowWidth, int iWindowHeight)
{
	if (!PsController::Instance().project) return;
	if (!PsController::Instance().project->pattern) return;

	PsProjectController& project = *(PsController::Instance().project);
	PsRender& renderer = PsController::Instance().project->renderer;

	//-- calculation of target coordinates 
	int max_width = iWindowWidth, max_height = iWindowHeight;
	float r1 = (float)project.GetWidth() / (float)max_width;
	float r2 = (float)project.GetHeight() / (float)max_height;
	if (r2 > r1) r1 = r2;
	this->dst_width = project.GetWidth() / r1;
	this->dst_height = project.GetHeight() / r1;
	int w_border = (iWindowWidth - this->dst_width) / 2;
	int h_border = 10; //(300 - dst_height) / 2;
	//--

	//-- update data
	this->dst_x1 = w_border, this->dst_y1 = h_border;
	this->dst_x2 = this->dst_x1 + this->dst_width, this->dst_y2 = this->dst_y1 + this->dst_height;
	//--

	this->m_RenduImage.Destroy();
	this->m_RenduImage.Create(this->dst_width, this->dst_height, 32);

	//-- centering the view on the matrix
	glPushMatrix();
	glViewport(0, 0, this->dst_width, this->dst_height);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, project.GetWidth(), project.GetHeight(), 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();
	//--

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef _WINDOWS
	//- gray background
	glColor4f(212.f / 255.f, 208.f / 255.f, 200.f / 255.f, 1.f);
	glBegin(GL_QUADS);
	glVertex2f(0, project.GetHeight());
	glVertex2f(project.GetWidth(), project.GetHeight());
	glVertex2f(project.GetWidth(), 0);
	glVertex2f(0, 0);
	glEnd();
	//--
#endif /* _WINDOWS */

	//-- drawing zones
	for (int i = 0; i < project.pattern->GetChannelsCount(); ++i)
	{
		if (i != project.iLayerId) glColor4f(0.75f, 0.75f, 1.00f, 0.8f);
		else glColor4f(0.f, 0.f, 0.6f, 0.8f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, project.pattern->aLayers[i]->iMaskTextureId);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(0, project.GetHeight());
		glTexCoord2f(1, 1);
		glVertex2f(project.GetWidth(), project.GetHeight());
		glTexCoord2f(1, 0);
		glVertex2f(project.GetWidth(), 0);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	glPopMatrix();
	//--

#ifdef _WINDOWS
	hardwareRenderer.CopyToSoftBuffer(this->m_RenduImage);
#else /* _MACOSX */
	hardwareRenderer.CopyToSoftBuffer(this->m_RenduImage, true);
#endif /* _WINDOWS */

}
