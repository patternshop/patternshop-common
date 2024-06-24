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
#pragma once

#include <map>

#include "PsWin.h"
#include "PsRender.h"
#include "PsHardware.h"
#include "PsController.h"

class PsScrollBar
{
public:
	virtual int GetWidth() = 0; 
	virtual void SetSize(int) = 0;
	virtual int GetPos() = 0; 
	virtual void Enable() = 0;	
	virtual void Disable() = 0;
};

class PsWinProjectModel
{
protected:
	PsWinProjectModel();
	virtual ~PsWinProjectModel();

public:
	enum OpenCloseState { OPEN = 1, CLOSE = 2 };
	typedef std::map<PsMatrix*, OpenCloseState> OpenCloseMap;
	
public: // ...
	int totalHSize;

protected:
	int matNameCount, motifNameCount, imageNameCount;
	PsShape *selected;
	static const int bloc_count_size, item_count_size;
	uint32 bloc_count, item_count;
	int ypos_precalc;
	bool bDragging, bDrawDragging;
	PsMatrix* dragLast;
	PsImage* dragBefore;
	bool dragTopmost;
	std::map<uint32, SoftwareBuffer*> imageList;
	SoftwareBuffer viewImage, boxImage;
	SoftwareBuffer directoryImage, openImage, closeImage;
	SoftwareBuffer directoryImageB, openImageB, closeImageB;
	SoftwareBuffer spot;
	PsRect s, w;
	OpenCloseMap openCloseMap;
	PsPoint draggingPoint, fromPoint;
	
public:
	PsCursor mouseCursor;
	PsScrollBar *scrollbar;
	
public:
	virtual void Show() = 0;
	virtual void Update() = 0;
	virtual void GenericUpdate() = 0;
	virtual void UpdateMouseCursor() = 0;
	virtual void OnLeftMouseButtonDown(PsPoint) = 0;
	virtual void OnLeftMouseButtonUp(PsPoint) = 0;
	virtual void OnMyMouseMove(PsPoint) = 0;
};

#ifdef _WINDOWS
#include "PsWinProjectWin32.h"
typedef PsWinProjectWin32 PsWinProjectView;
#else /* _MACOSX */
#include "PsWinProjectMac.h"
typedef PsWinProjectMac PsWinProjectView;
#endif


class PsWinProject : public PsWinProjectView
{
protected:
	PsWinProject();
	
public:
	static PsWinProject& Instance();
	static void Delete();
   virtual ~PsWinProject();

protected:
	static PsWinProject* instance; // Singleton
	
public:
	void OnLeftMouseButtonDown(PsPoint);
	void OnLeftMouseButtonUp(PsPoint);
	void OnMyMouseMove(PsPoint);

protected:
	void OnLButtonDownIn(PsPoint, PsImage*);
	SoftwareBuffer *loadThumb(PsTexture*);
	void DrawMatrixBloc(PsMatrix *);
	void DrawImageBloc(PsImage *);
	void DrawBackgroundBloc();
	void DrawInsertionCaret();
	
public:
	void GenericUpdate();
	void relaseThumb(PsTexture*);
};
