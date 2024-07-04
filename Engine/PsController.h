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

#include "PsMessage.h"

#ifdef WIN32
# include "stdafx.h"
class	CPatternshopView;
#else // _MACOSX
class PsMacView;
#endif

class	PsProject;

#define CONTROLLER_COUNT_CURSOR	17		// Nombre de curseurs définis
#define CONTROLLER_COUNT_OPTION	8		// Nombre d'options disponibles

enum PsCursor	// Identifiants des curseurs
{
	CURSOR_DEFAULT,
	CURSOR_MAGNIFY1,
	CURSOR_MAGNIFY3,
	CURSOR_MAGNIFY2,
	CURSOR_MOVE,
	CURSOR_ROTATE1,
	CURSOR_ROTATE2,
	CURSOR_ROTATE3,
	CURSOR_ROTATE4,
	CURSOR_SCROLL1,
	CURSOR_SCROLL2,
	CURSOR_SIZE1,
	CURSOR_SIZE2,
	CURSOR_TORSIO1,
	CURSOR_TORSIO2,
	CURSOR_FINGER,
	CURSOR_DRAG
};

/*
** PsController : Singleton, donc commun à tous les projets ouverts; il fait la liaison
** entre l'interface graphique et la classe "PsProject" qui représente un seul projet.
*/
class	PsController
{
public:

	enum	Tool	// Outil actif, et son éventuel mode (certains outils peuvent avoir plusieurs modes
		// de fonctionnement, comme l'outil "modify" qui sert à déplacer, redimentionner...)
	{
		TOOL_MAGNIFY,
		TOOL_MAGNIFY_ZOOM,
		TOOL_MODIFY,
		TOOL_MODIFY_MOVE,
		TOOL_MODIFY_ROTATE,
		TOOL_MODIFY_SIZE,
		TOOL_MODIFY_TORSIO,
		TOOL_SCROLL,
		TOOL_SCROLL_DRAG,
	};

	enum	Option	// Identifiants des options
	{
		OPTION_AUTOMATIC,
		//OPTION_BOX_MOVE,
		OPTION_BOX_SHOW,
		OPTION_CONSTRAIN,
		OPTION_DOCUMENT_BLEND,
		OPTION_DOCUMENT_SHOW,
		OPTION_HIGHLIGHT_SHOW,
		OPTION_REFLECT
	};

	~PsController();

	static PsController&	Instance();
	static void				Delete();

	bool					GetOption(Option, bool = false) const;

	void					MouseClick(int, int, int);
	void					MouseMove(int, int);
	void					MouseRelease(int, int, int);

	void					UpdateWindow();
	void					UpdateDialogProject();
	void					UpdateDialogOverview(bool bQuick = true);

#ifndef _MACOSX
	void					SetActive(CPatternshopView*);
#else
	void					SetActive(PsProject*);
#endif
	void					SetCursor(PsCursor);
	void					SetOption(Option, bool);
	void					SetProgress(int);
	void					SetTool(Tool);

	Tool					tool, prev_tool;					// Outil actuellement sélectionné
	PsProject*				project;

public:
	PsController();

#ifdef WIN32
	HCURSOR					cursor[CONTROLLER_COUNT_CURSOR];	// Tableau des curseurs
	CPatternshopView*		active;								// Fenêtre active de l'application
#endif

	static PsController*	instance;							// Instance de PsController(singleton)
	// Projet actif(lié à "active")
	bool					option[CONTROLLER_COUNT_OPTION];	// Valeurs des options
	int						prev_x;								// Utilisé en interne pour sauver la position
	int						prev_y;								//   du pointeur lors d'un clic.

	bool					bMouseButtonIsDown;
};
