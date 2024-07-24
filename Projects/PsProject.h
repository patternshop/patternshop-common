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

#include <list>
#include <math.h>
#include "PsRender.h"
#include "PsMessage.h"
#include "PsController.h"
#include "PsMatrix.h"

class	PsAction;
class	PsMatrix;

typedef std::list<PsAction*>	LogList;
typedef std::list<PsMatrix*>	MatrixList;

// Project parameters
#define ZOOM_COEF        0.02f
#define LOG_SIZE         50

/*
** Cette classe représente un projet, donc principalement une liste de shapes (qui elles-même contiennent
** des images), une liste de patrons, et un document qui est symbolisé par la classe PsRender (la seule chose
** utile à propos du document est sa taille, et elle est contenue dans PsRender qui en a besoin pour
** l'affichage).
 */
class	PsProjectController
{
	friend class		PsRender;

public:
	PsProjectController();
	~PsProjectController();

public:
	ErrID				CloneImage();
	ErrID				CloneMatrix();
	ErrID				ReplaceImage(const char*);
	ErrID				DelImage();
	ErrID				DelMatrix();
	ErrID				DelPattern();
	ErrID				NewImage(const char*);
	ErrID				NewMotif(const char*);
	ErrID				NewMatrix();
	ErrID				NewPattern(const char*);
	ErrID				MatrixColor();
	ErrID				MatrixReset();

public:
	void				LogFlush();
	void				LogAdd(PsAction*);
	bool				LogCanRedo() const;
	bool				LogCanUndo() const;
	void				LogInit();
	int					LogUndoCount();
	const char*			LogRedoLastName() const;
	const char*			LogUndoLastName() const;
	bool				LogMustAdd() const;
	void				LogRedo();
	void				LogUndo();

public:
	void  RenderToScreen();

public:
	bool  RenderToFile(const char*, int, int);

public:
	void  SelectImage(PsImage*);
	void  SelectMatrix(PsMatrix*);
	void  ToolMagnifyDrag(int, int, int);
	void  ToolModifyMove(int, int, int, int, PsController::Tool);
	void  ToolScrollDrag(int, int, int, int);

public:
	PsController::Tool	ToolMagnifyStart();
	PsController::Tool	ToolModifyScan(int, int, bool);
	PsController::Tool	ToolScrollStart();

public:
	int GetWidth();
	int GetHeight();
	int GetDpi();

public:
	enum	Mode
	{
		MODE_DEFAULT,
		MODE_MOVE,
		MODE_ROTATE,
		MODE_SIZE,
		MODE_TORSIO
	};

public:
	PsPattern*		pattern;
	MatrixList		matrices;
	ImageList		images;

public:
	int					iColor[3];
	bool				bHideColor;

public:
	PsImage*		image;
	PsMatrix*		matrix;
	PsShape*		shape;
	PsRender		renderer;

public:
	bool				bNeedSave;
	bool				bPatternsIsSelected;
	int					iLayerId;
	Mode				mode;
	bool				center;

private:
	bool				log_insert;
	LogList				log_redo;
	LogList				log_undo;
	int					init_corner;

	// variables temporaires utilisées pour la manipulation à la souris 
	float				prev_scrollx;
	float				prev_scrolly;
	float				prev_zoom;
	float				prev_h;
	float				prev_i;
	float				prev_j;
	float				prev_r;
	float				prev_w;
	float				prev_x;
	float				prev_y;
	PsVector			prev_origin;
	PsVector			prev_origin_z0;
};

