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

class PsWinProject
{
public:
	PsWinProject(PsWin* psWin);
	virtual ~PsWinProject();

public:
	static void Delete();

public:
	enum OpenCloseState { OPEN = 1, CLOSE = 2 };
	typedef std::map<PsMatrix*, OpenCloseState> OpenCloseMap;

public: // ...
	int totalHSize;

protected:
	PsWin* psWin;
	int matNameCount, motifNameCount, imageNameCount;
	PsShape* selected;
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
	PsScrollBar* scrollbar;

public:
	virtual void Show() = 0;
	virtual void Update() = 0;
	virtual void UpdateMouseCursor() = 0;

protected:
	static PsWinProject* instance; // Singleton

public:
	static PsWinProject& Instance();
	static void setInstance(PsWinProject* instance);

public:
	virtual void OnLeftMouseButtonDown(PsPoint);
	virtual void OnLeftMouseButtonUp(PsPoint);
	virtual void OnMyMouseMove(PsPoint);

protected:
	void OnLButtonDownIn(PsPoint, PsImage*);
	SoftwareBuffer* loadThumb(PsTexture*);
	void DrawMatrixBloc(PsMatrix*);
	void DrawImageBloc(PsImage*);
	void DrawBackgroundBloc();
	void DrawInsertionCaret();

public:
	virtual void GenericUpdate();
	void relaseThumb(PsTexture*);
};
