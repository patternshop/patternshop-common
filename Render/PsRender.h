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
#ifndef PS_RENDER_H__

#define PS_RENDER_H__

#include "PsOpenGL.h"

#include "PsTypes.h"
#include "PsImage.h"
#include "PsMatrix.h"
#include "PsPattern.h"

#define RENDERER_INFINITE	(1e16f)
#define RENDERER_ZOOM_INIT	1.1f
#define	RENDERER_BACKGROUND	(1 / 16.0f)

class	PsProjectController;

/*
** La classe PsRender s'occupe du rendu hardware et software du projet.
*/
class	PsRender
{
	friend class PsProjectController;

	/*
	** Enumérations
	*/

public:

	enum	Engine
	{
		ENGINE_HARDWARE,
		ENGINE_SOFTWARE
	};

	/*
	** Méthodes
	*/

public:

	PsRender();
	virtual			~PsRender();

	void			CenterView();
	void			Convert(int, int, float&, float&) const;

	void			Render(PsProjectController&, int, int);

	void			MonoLayerRendering(PsProjectController&, int, int);
	void			MultiLayerRendering(PsProjectController&, int, int);

	uint8* GetBuffer(int, int) const;
	void			GetDocSize(int&, int&) const;
	void			GetScroll(float&, float&) const;
	void			GetSize(int&, int&) const;

	void			Recalc();
	void			SetDocSize(int, int);
	void			SetEngine(Engine);
	void			SetScroll(float, float);
	void			SetSize(int, int);
	void			SetZone(float, float);
	void			SetZoom(float);

	void			PrepareSurface(PsProjectController&, int, int);

	bool			IsInside(const PsImage&, int, int) const;
	inline bool		IsInside(int, int) const;

	void			GetMatrixWindow(PsMatrix&, double&, double&, double&, double&);

	void			DrawStandardGizmos(PsProjectController&);
	void			DrawPatternGizmo(PsPattern&, int);
	void			DrawBox(const PsImage&);
	void			DrawBox(const PsMatrix&);
	void			DrawBoxHandle(const PsMatrix&);
	void			DrawMatricesGizmos(PsProjectController&);
	void			DrawGizmos(PsProjectController&);

	void			DrawDocument();
	void			DrawBack(const PsProjectController&, float, float);
	void			DrawPattern(PsPattern&/*, bool*/);
	void			DrawShape(PsImage&, float x = 0, float y = 0, bool first = true, bool last = true);
	void			DrawShape(PsMatrix&);
	void			DrawMatrices(PsProjectController&);
	void			DrawImages(PsProjectController&);
	void			DrawLayerTexture(GLuint);

	PsVector		GetEyeLocation();
	GLuint			CreateDocumentTexture(PsProjectController&);
	void			UpdateLayerTexture(PsProjectController&, PsLayer*, GLuint);


	/*
	** Variables
	*/

public:

	Engine			engine;
	PsTexture		back;

	int				doc_x;
	int				doc_y;
	float			scroll_x;
	float			scroll_y;
	int				size_x;
	int				size_y;
	float			x1;
	float			x2;
	float			y1;
	float			y2;
	float			zoom;
	int				dpi;

	float			fZoomMax;
	float			fZoomMin;

	int				iLayerTextureSize;
};

#endif /* PS_RENDER_H__ */

