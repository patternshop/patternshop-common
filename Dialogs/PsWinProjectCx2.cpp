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
#include "PsAction.h"
#include "PsController.h"
#include "PsWinProject.h"
#include "PsDlgInfoLight.h"
#include "PsDlgColor.h"
#include "FreeImage.h"

#ifdef _MACOSX
#include "PsMacTools.h"
#endif

#include <assert.h>

void PsWinProjectCx::OnLeftMouseButtonUp(PsPoint point)
{
	PsProjectController* project_controller = PsController::Instance().project_controller;
	if (!project_controller) return;
	if (this->bDragging)
	{
		this->bDragging = false;
		if (dynamic_cast<PsImage*>(this->selected))
		{
			if (this->dragBefore)
			{
				PsImage* imageSource = (PsImage*)this->selected;
				PsMatrix* matrixSource = (PsMatrix*)imageSource->parent;
				PsMatrix* matrixDest = (PsMatrix*)this->dragBefore->parent;
				if (imageSource)
				{
					// > log
					ImageList::iterator t;
					int j;
					if (matrixSource)
					{
						for (j = 0, t = matrixSource->images.begin(); t != matrixSource->images.end() && *t != imageSource; ++t)
							++j;
						if (j == matrixSource->images.size())
							j = -1;
					}
					else
					{
						for (j = 0, t = project_controller->images.begin(); t != project_controller->images.end() && *t != imageSource; ++t)
							++j;
						if (j == project_controller->images.size())
							j = -1;
					}
					// < log
					if (matrixSource) matrixSource->images.remove(imageSource);
					else project_controller->images.remove(imageSource);
					imageSource->parent = matrixDest;
					ImageList* images = &project_controller->images;
					if (matrixDest) images = &matrixDest->images;
					ImageList::iterator i = images->begin();
					bool operation_successful = false;
					for (; i != images->end(); ++i)
					{
						if (*i == this->dragBefore)
						{
							++i;
							images->insert(i, imageSource);
							operation_successful = true;
							break;
						}
					}
					if (!operation_successful) // Failsafe
						images->push_back(imageSource);
					if (imageSource->parent)
					{
						imageSource->x = 0;
						imageSource->y = 0;
					}
					else if (matrixDest || matrixSource)
						imageSource->SetPosition(project_controller->GetWidth() / 2, project_controller->renderer.doc_y / 2);
					project_controller->LogAdd(new LogSwapImage(*project_controller, imageSource, matrixDest, matrixSource, j));
					project_controller->SelectImage(imageSource);
				}
			}
			else if (this->dragLast)
			{
				PsImage* imageSource = (PsImage*)this->selected;
				PsMatrix* matrixSource = (PsMatrix*)imageSource->parent;
				PsMatrix* matrixDest = (PsMatrix*)this->dragLast;
				if (imageSource)
				{
					// > log
					ImageList::iterator t;
					int j;
					if (matrixSource)
					{
						for (j = 0, t = matrixSource->images.begin(); t != matrixSource->images.end() && *t != imageSource; ++t)
							++j;
						if (j == matrixSource->images.size())
							j = -1;
					}
					else
					{
						for (j = 0, t = project_controller->images.begin(); t != project_controller->images.end() && *t != imageSource; ++t)
							++j;
						if (j == project_controller->images.size())
							j = -1;
					}
					// < log
					if (matrixSource) matrixSource->images.remove(imageSource);
					else project_controller->images.remove(imageSource);
					imageSource->parent = matrixDest;
					matrixDest->images.insert(matrixDest->images.begin(), imageSource);
					if (imageSource->parent)
					{
						imageSource->x = 0;
						imageSource->y = 0;
					}
					else if (matrixDest || matrixSource)
						imageSource->SetPosition(project_controller->GetWidth() / 2, project_controller->renderer.doc_y / 2);
					project_controller->LogAdd(new LogSwapImage(*project_controller, imageSource, matrixDest, matrixSource, j));
					project_controller->SelectImage(imageSource);
				}
			}
			else if (this->dragTopmost)
			{
				PsImage* imageSource = (PsImage*)this->selected;
				PsMatrix* matrixSource = (PsMatrix*)imageSource->parent;
				if (imageSource)
				{
					// > log
					ImageList::iterator t;
					int j;
					if (matrixSource)
					{
						for (j = 0, t = matrixSource->images.begin(); t != matrixSource->images.end() && *t != imageSource; ++t)
							++j;
						if (j == matrixSource->images.size())
							j = -1;
					}
					else
					{
						for (j = 0, t = project_controller->images.begin(); t != project_controller->images.end() && *t != imageSource; ++t)
							++j;
						if (j == project_controller->images.size())
							j = -1;
					}
					// < log
					if (matrixSource) matrixSource->images.remove(imageSource);
					else project_controller->images.remove(imageSource);
					imageSource->parent = NULL;
					project_controller->images.push_back(imageSource);
					if (imageSource->parent)
					{
						imageSource->x = 0;
						imageSource->y = 0;
					}
					else if (matrixSource)
						imageSource->SetPosition(project_controller->GetWidth() / 2, project_controller->renderer.doc_y / 2);
					project_controller->LogAdd(new LogSwapImage(*project_controller, imageSource, 0, matrixSource, j));
					project_controller->SelectImage(imageSource);
				}
			}
		}
		else
		{
			if (this->dragLast && project_controller)
			{
				PsMatrix* matrixSource = (PsMatrix*)this->selected;
				PsMatrix* matrixDest = (PsMatrix*)this->dragLast;
				if (matrixSource && matrixDest && matrixSource != matrixDest)
				{
					// > log
					MatrixList::iterator t;
					int j;
					for (j = 0, t = project_controller->matrices.begin(); t != project_controller->matrices.end() && *t != matrixSource; ++t)
						++j;
					if (j == project_controller->matrices.size())
						j = -1;
					// < log
					project_controller->matrices.remove(matrixSource);
					MatrixList::iterator i = project_controller->matrices.begin();
					bool operation_successful = false;
					for (; i != project_controller->matrices.end(); ++i)
					{
						if (*i == matrixDest)
						{
							project_controller->matrices.insert(i, matrixSource);
							operation_successful = true;
							break;
						}
					}
					if (!operation_successful) // Failsafe
						project_controller->matrices.push_back(matrixSource);
					project_controller->LogAdd(new LogSwapMatrix(*project_controller, matrixSource, j));
				}
			}
		}
		this->dragLast = NULL;
		this->dragBefore = NULL;
		this->dragTopmost = false;
		this->OnMyMouseMove(point);
		this->Update();
		PsController::Instance().UpdateWindow();
	}
	//	if (PsController::Instance().active)
	//		PsController::Instance().active->SetFocus();
}

void PsWinProjectCx::OnMyMouseMove(PsPoint point)
{
	//scrollbar->GetClientRect(&s);
	if (!this->bDragging)
	{
		if (point.y > 0 && point.x < this->psWin->iWidth - this->scrollbar->GetWidth()
			&& point.y < this->totalHSize - this->scrollbar->GetPos())
		{
			if (this->mouseCursor != CURSOR_FINGER)
			{
				this->mouseCursor = CURSOR_FINGER;
				this->UpdateMouseCursor();
			}
		}
		else
		{
			if (this->mouseCursor != CURSOR_DEFAULT)
			{
				this->mouseCursor = CURSOR_DEFAULT;
				this->UpdateMouseCursor();
			}
		}
	}
	else
	{
		if (abs(this->fromPoint.y - point.y) > 2)
		{
			this->bDrawDragging = true;
			this->mouseCursor = CURSOR_SCROLL2;
			this->UpdateMouseCursor();
		}
		this->draggingPoint = point;
		this->Update();
	}
}

SoftwareBuffer* PsWinProjectCx::loadThumb(PsTexture* g)
{
	//-- transformation into the manipulation format
	int bpp;
	int size_x = g->width, size_y = g->height;
	uint8* buffer = g->GetBufferUncompressed(bpp);
	FIBITMAP* im = FreeImage_FromBuffer(buffer, size_x, size_y, bpp);
	delete[] buffer;
	//--

	//-- calculate the ratio
	int tw = 25, th = 20;
	double ratio1 = size_x / tw, ratio2 = size_y / th;
	if (ratio1 < ratio2) ratio1 = ratio2;
	if (ratio1 <= 0)
	{
		tw = size_x;
		th = size_y;
	}
	else
	{
		tw = (int)(size_x / ratio1);
		th = (int)(size_y / ratio1);
	}
	//--

	//-- create the thumbnail
	FIBITMAP* thumbnail = FreeImage_Rescale(im, tw, th, FILTER_BOX);
	FreeImage_Unload(im);
	im = thumbnail;
	//--

	//-- copy on white background
	thumbnail = FreeImage_CreateWhiteRGBA(tw, th);
	FreeImage_BlendPaste(thumbnail, im, 0, 0);
	FreeImage_Unload(im);
	im = thumbnail;
	//--

	//-- transform into SoftwareBuffer
	SoftwareBuffer* img = new SoftwareBuffer;
	img->Create(tw, th, 24);
	int bytespp = FreeImage_GetLine(im) / FreeImage_GetWidth(im);
	for (int j = 0; j < th; ++j)
	{
		BYTE* bits = FreeImage_GetScanLine(im, j);
		for (int i = 0; i < tw; ++i)
		{
#ifdef _WINDOWS
			img->buffer.SetPixelRGB(i, j, bits[FI_RGBA_RED], bits[FI_RGBA_GREEN], bits[FI_RGBA_BLUE]);
#else /* _MACOSX */
			int p = (i + j * tw) * 4;
			img->bits[p++] = 0;
			img->bits[p++] = bits[FI_RGBA_RED];
			img->bits[p++] = bits[FI_RGBA_GREEN];
			img->bits[p++] = bits[FI_RGBA_BLUE];
#endif
			bits += bytespp;
		}
	}
	//--

	this->imageList[g->GetAutoGenId()] = img;

	FreeImage_Unload(im);

	return img;
}

void PsWinProjectCx::relaseThumb(PsTexture* texture)
{
	if (this->imageList.find(texture->GetAutoGenId()) != this->imageList.end())
	{
		SoftwareBuffer* img = this->imageList[texture->GetAutoGenId()];
		if (img)
		{
			delete img;
			this->imageList[texture->GetAutoGenId()] = NULL;
		}
		this->imageList.erase(texture->GetAutoGenId());
	}
}
