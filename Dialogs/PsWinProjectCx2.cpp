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
	PsProject* project = PsController::Instance().project;
	if (!project) return;
	if (bDragging)
	{
		bDragging = false;
		if (dynamic_cast<PsImage*>(selected))
		{
			if (dragBefore)
			{
				PsImage* imageSource = (PsImage*)selected;
				PsMatrix* matrixSource = (PsMatrix*)imageSource->parent;
				PsMatrix* matrixDest = (PsMatrix*)dragBefore->parent;
				if (imageSource)
				{
					// > log
					ImageList::iterator	t;
					int					j;
					if (matrixSource)
					{
						for (j = 0, t = matrixSource->images.begin(); t != matrixSource->images.end() && *t != imageSource; ++t)
							++j;
						if (j == matrixSource->images.size())
							j = -1;
					}
					else
					{
						for (j = 0, t = project->images.begin(); t != project->images.end() && *t != imageSource; ++t)
							++j;
						if (j == project->images.size())
							j = -1;
					}
					// < log
					if (matrixSource) matrixSource->images.remove(imageSource);
					else project->images.remove(imageSource);
					imageSource->parent = matrixDest;
					ImageList* images = &project->images;
					if (matrixDest) images = &matrixDest->images;
					ImageList::iterator i = images->begin();
					bool operation_succesfull = false;
					for (; i != images->end(); ++i)
					{
						if (*i == dragBefore)
						{
							++i;
							images->insert(i, imageSource);
							operation_succesfull = true;
							break;
						}
					}
					if (!operation_succesfull) // Garde fou
						images->push_back(imageSource);
					if (imageSource->parent)
					{
						imageSource->x = 0;
						imageSource->y = 0;
					}
					else if (matrixDest || matrixSource)
						imageSource->SetPosition(project->GetWidth() / 2, project->renderer.doc_y / 2);
					project->LogAdd(new LogSwapImage(*project, imageSource, matrixDest, matrixSource, j));
					project->SelectImage(imageSource);
				}
			}
			else if (dragLast)
			{
				PsImage* imageSource = (PsImage*)selected;
				PsMatrix* matrixSource = (PsMatrix*)imageSource->parent;
				PsMatrix* matrixDest = (PsMatrix*)dragLast;
				if (imageSource)
				{
					// > log
					ImageList::iterator	t;
					int					j;
					if (matrixSource)
					{
						for (j = 0, t = matrixSource->images.begin(); t != matrixSource->images.end() && *t != imageSource; ++t)
							++j;
						if (j == matrixSource->images.size())
							j = -1;
					}
					else
					{
						for (j = 0, t = project->images.begin(); t != project->images.end() && *t != imageSource; ++t)
							++j;
						if (j == project->images.size())
							j = -1;
					}
					// < log
					if (matrixSource) matrixSource->images.remove(imageSource);
					else project->images.remove(imageSource);
					imageSource->parent = matrixDest;
					matrixDest->images.insert(matrixDest->images.begin(), imageSource);
					if (imageSource->parent)
					{
						imageSource->x = 0;
						imageSource->y = 0;
					}
					else if (matrixDest || matrixSource)
						imageSource->SetPosition(project->GetWidth() / 2, project->renderer.doc_y / 2);
					project->LogAdd(new LogSwapImage(*project, imageSource, matrixDest, matrixSource, j));
					project->SelectImage(imageSource);
				}
			}
			else if (dragTopmost)
			{
				PsImage* imageSource = (PsImage*)selected;
				PsMatrix* matrixSource = (PsMatrix*)imageSource->parent;
				if (imageSource)
				{
					// > log
					ImageList::iterator	t;
					int					j;
					if (matrixSource)
					{
						for (j = 0, t = matrixSource->images.begin(); t != matrixSource->images.end() && *t != imageSource; ++t)
							++j;
						if (j == matrixSource->images.size())
							j = -1;
					}
					else
					{
						for (j = 0, t = project->images.begin(); t != project->images.end() && *t != imageSource; ++t)
							++j;
						if (j == project->images.size())
							j = -1;
					}
					// < log
					if (matrixSource) matrixSource->images.remove(imageSource);
					else project->images.remove(imageSource);
					imageSource->parent = NULL;
					project->images.push_back(imageSource);
					if (imageSource->parent)
					{
						imageSource->x = 0;
						imageSource->y = 0;
					}
					else if (matrixSource)
						imageSource->SetPosition(project->GetWidth() / 2, project->renderer.doc_y / 2);
					project->LogAdd(new LogSwapImage(*project, imageSource, 0, matrixSource, j));
					project->SelectImage(imageSource);
				}
			}
		}
		else
		{
			if (dragLast && project)
			{
				PsMatrix* matrixSource = (PsMatrix*)selected;
				PsMatrix* matrixDest = (PsMatrix*)dragLast;
				if (matrixSource && matrixDest && matrixSource != matrixDest)
				{
					// > log
					MatrixList::iterator	t;
					int						j;
					for (j = 0, t = project->matrices.begin(); t != project->matrices.end() && *t != matrixSource; ++t)
						++j;
					if (j == project->matrices.size())
						j = -1;
					// < log
					project->matrices.remove(matrixSource);
					MatrixList::iterator i = project->matrices.begin();
					bool operation_succesfull = false;
					for (; i != project->matrices.end(); ++i)
					{
						if (*i == matrixDest)
						{
							project->matrices.insert(i, matrixSource);
							operation_succesfull = true;
							break;
						}
					}
					if (!operation_succesfull) // Garde fou
						project->matrices.push_back(matrixSource);
					project->LogAdd(new LogSwapMatrix(*project, matrixSource, j));
				}
			}
		}
		dragLast = NULL;
		dragBefore = NULL;
		dragTopmost = false;
		OnMyMouseMove(point);
		Update();
		PsController::Instance().UpdateWindow();
	}
	//	if (PsController::Instance().active)
	//		PsController::Instance().active->SetFocus();
}


void PsWinProjectCx::OnMyMouseMove(PsPoint point)
{
	//scrollbar->GetClientRect(&s);
	if (!bDragging)
	{
		if (point.y > 0 && point.x < psWin->iWidth - scrollbar->GetWidth()
			&& point.y < totalHSize - scrollbar->GetPos())
		{
			if (mouseCursor != CURSOR_FINGER)
			{
				mouseCursor = CURSOR_FINGER;
				UpdateMouseCursor();
			}
		}
		else
		{
			if (mouseCursor != CURSOR_DEFAULT)
			{
				mouseCursor = CURSOR_DEFAULT;
				UpdateMouseCursor();
			}
		}
	}
	else
	{
		if (abs(fromPoint.y - point.y) > 2)
		{
			bDrawDragging = true;
			mouseCursor = CURSOR_SCROLL2;
			UpdateMouseCursor();
		}
		draggingPoint = point;
		Update();
	}
}

SoftwareBuffer* PsWinProjectCx::loadThumb(PsTexture* g)
{
	//-- transformation dans le format de manipulation
	int bpp;
	int size_x = g->width, size_y = g->height;
	uint8* buffer = g->GetBufferUncompressed(bpp);
	FIBITMAP* im = FreeImage_FromBuffer(buffer, size_x, size_y, bpp);
	delete[] buffer;
	//--

	//-- calcul du ratio
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

	//-- creation de la miniature
	FIBITMAP* thumbnail = FreeImage_Rescale(im, tw, th, FILTER_BOX);
	FreeImage_Unload(im);
	im = thumbnail;
	//--

	//-- copie sur fond blanc
	thumbnail = FreeImage_CreateWhiteRGBA(tw, th);
	FreeImage_BlendPaste(thumbnail, im, 0, 0);
	FreeImage_Unload(im);
	im = thumbnail;
	//--

	//-- transformation en SoftwareBuffer
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

	imageList[g->GetAutoGenId()] = img;

	FreeImage_Unload(im);

	return img;
}

void PsWinProjectCx::relaseThumb(PsTexture* texture)
{
	if (imageList.find(texture->GetAutoGenId()) != imageList.end())
	{
		SoftwareBuffer* img = imageList[texture->GetAutoGenId()];
		if (img)
		{
			delete img;
			imageList[texture->GetAutoGenId()] = NULL;
		}
		imageList.erase(texture->GetAutoGenId());
	}
}

