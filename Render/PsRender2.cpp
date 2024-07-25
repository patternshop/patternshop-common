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
 ** Gets the orthonormal dimensions of a matrix
 */
void  PsRender::GetMatrixWindow(PsMatrix& matrix, double& left, double& right, double& bottom, double& top)
{
	float corner[4][4];
	matrix.ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
	matrix.ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
	matrix.ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
	matrix.ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);
	left = corner[0][0];
	for (int i = 0; i < 4; ++i)
		if (corner[i][0] < left)
			left = corner[i][0];
	right = corner[0][0];
	for (int i = 0; i < 4; ++i)
		if (corner[i][0] > right)
			right = corner[i][0];
	bottom = corner[0][1];
	for (int i = 0; i < 4; ++i)
		if (corner[i][1] > bottom)
			bottom = corner[i][1];
	top = corner[0][1];
	for (int i = 0; i < 4; ++i)
		if (corner[i][1] < top)
			top = corner[i][1];
}


/*
 ** Draws the content of a matrix (the matrix itself is not representable, except possibly
 ** a colored frame to indicate the central matrix), and repeats this display across the screen to
 ** draw a complete network. In the end, this function only calls DrawShape for each image,
 ** as many times as necessary.
 */
void  PsRender::DrawShape(PsMatrix& matrix)
{
	if (matrix.hide)
		return;

	//-- size of the smallest side of the matrix
	double minSize = matrix.w;
	if (minSize > matrix.h)
		minSize = matrix.h;
	//--

	//-- size of the diagonal of the rendering area
	double l, r, b, t;
	GetMatrixWindow(matrix, l, r, b, t);
	if (this->x2 > r) r = this->x2;
	if (this->y1 > b) b = this->y1;
	if (this->x1 < l) l = this->x1;
	if (this->y2 < t) t = this->y2;
	double dw = (r - l);
	double dh = (b - t);
	double powDiag = sqrt(pow(dw, 2) + pow(dh, 2));
	//--

	//-- double the min (FIXME, it's quick and dirty)
	double quickDup = round((powDiag / minSize) * 2);
	double quickDupMax = 300;
	if (quickDup > quickDupMax)
	{
		//warning ?
		quickDup = quickDupMax;
	}
	//--

	int iMaximum = DoubleToInt(quickDup);

	float cos_r = cos(matrix.r);
	float sin_r = sin(matrix.r);
	float i2 = matrix.i;
	float j2 = matrix.j;
	float mh = matrix.h;
	float mw = matrix.w;

	glColor4f(1.f, 1.f, 1.f, 1.f);
	ImageList::const_iterator image;
	for (image = matrix.images.begin(); image != matrix.images.end(); ++image)
	{
		bool bFirst = true;
		for (int y = -iMaximum; y < iMaximum; ++y)
		{
			float yi2 = y * i2;
			float ymh = y * mh;
			for (int x = -iMaximum; x <= iMaximum; ++x)
			{
				float xj2 = x * j2;
				float xmw = x * mw;
				float tx = (xmw + yi2) * cos_r - (ymh + xj2) * sin_r;
				float ty = (xmw + yi2) * sin_r + (ymh + xj2) * cos_r;
				DrawShape(**image, tx, ty, bFirst, false);
				if (bFirst) bFirst = false;
			}
		}
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}

/*
 ** Returns the buffer in which the software rendering was done (for now, it's a capture
 ** of the hardware rendering, which in addition to being a bad solution in terms of quality, prevents
 ** capturing an area larger than the screen).
 */
uint8* PsRender::GetBuffer(int x, int y) const
{
	uint8* buffer = new uint8[x * y * 4];
	uint8	pixel;
	int				i;
	int				j;

	glFlush();
	glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	for (i = 0, j = x * y * 4; i < j; i += 4)
	{
		pixel = buffer[i + 0];
		buffer[i + 0] = buffer[i + 2];
		buffer[i + 2] = pixel;
	}

	return buffer;
}

/*
 ** Gets the document size.
 */
void	PsRender::GetDocSize(int& x, int& y) const
{
	x = this->doc_x;
	y = this->doc_y;
}

/*
 ** Gets the current scrolling values.
 */
void	PsRender::GetScroll(float& x, float& y) const
{
	x = this->scroll_x;
	y = this->scroll_y;
}

/*
 ** Gets the size of the view area.
 */
void	PsRender::GetSize(int& x, int& y) const
{
	x = this->size_x;
	y = this->size_y;
}

/*
 ** Recalculates the coordinates of the window edges from the document's point of view, based on
 ** scrolling, zoom, and window size.
 */
void		PsRender::Recalc()
{
	float	zx = this->size_x * this->zoom / 2;
	float	zy = this->size_y * this->zoom / 2;

	this->x1 = this->scroll_x - zx;
	this->x2 = this->scroll_x + zx;
	this->y1 = this->scroll_y + zy;
	this->y2 = this->scroll_y - zy;
}

void PsRender::PrepareSurface(PsProjectController& project_controller, int x, int y)
{
	float fMaxSize = this->doc_x > this->doc_y ? this->doc_x : this->doc_y;

	float fMaxWidth, hMaxHeight, fD1;
	fMaxWidth = fMaxSize;
	hMaxHeight = fMaxSize;
	fD1 = hMaxHeight / 4.f;

	if (this->engine == ENGINE_HARDWARE)
	{

		glViewport(0, 0, x, y);
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(this->x1, this->x2, this->y1, this->y2, -10000, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPopMatrix();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}


void PsRender::DrawMatrices(PsProjectController& project_controller)
{
	MatrixList::const_iterator i;
	int n;

	for (n = 0, i = project_controller.matrices.begin(); i != project_controller.matrices.end(); ++i)
	{
		if (this->engine == ENGINE_SOFTWARE)
			PsController::Instance().SetProgress((int)(20 + 60 * n++ / project_controller.matrices.size()));
		this->DrawShape(**i);
	}
}

void PsRender::DrawImages(PsProjectController& project_controller)
{
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	ImageList::const_iterator j;
	for (j = project_controller.images.begin(); j != project_controller.images.end(); ++j)
		this->DrawShape(**j);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void PsRender::DrawMatricesGizmos(PsProjectController& project_controller)
{
	MatrixList::const_iterator i;

	for (i = project_controller.matrices.begin(); i != project_controller.matrices.end(); ++i)
	{
		PsMatrix& matrix = **i;
		if (!matrix.hide)
		{
			this->DrawBox(matrix);
			glColor4f(1.f, 1.f, 1.f, 0.8f);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			ImageList::const_iterator image;
			for (image = matrix.images.begin(); image != matrix.images.end(); ++image)
				this->DrawShape(**image);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
}

void PsRender::DrawPatternGizmo(PsPattern& pattern, int iIndex)
{
	PsLayer* layer = pattern.aLayers[iIndex];

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//-- layer grid
	layer->UpdateGizmoProjection(*this);
	PsVector* vCorners = layer->vProjected;
	glColor4f(0.0f, 0.0f, 0.2f, 0.5f);
	float p[4];
	float v[4];
	float div_x = 8, div_y = 8;
	v[0] = (vCorners[1].X - vCorners[0].X) / (float)div_x;
	v[1] = (vCorners[1].Y - vCorners[0].Y) / (float)div_x;
	v[2] = (vCorners[2].X - vCorners[3].X) / (float)div_x;
	v[3] = (vCorners[2].Y - vCorners[3].Y) / (float)div_x;
	for (int i = 0; i <= div_x; ++i)
	{
		p[0] = vCorners[0].X + v[0] * i;
		p[1] = vCorners[0].Y + v[1] * i;
		p[2] = vCorners[3].X + v[2] * i;
		p[3] = vCorners[3].Y + v[3] * i;
		glBegin(GL_LINE_LOOP);
		glVertex2f(p[0], p[1]);
		glVertex2f(p[2], p[3]);
		glEnd();
	}
	v[0] = (vCorners[3].X - vCorners[0].X) / (float)div_y;
	v[1] = (vCorners[3].Y - vCorners[0].Y) / (float)div_y;
	v[2] = (vCorners[2].X - vCorners[1].X) / (float)div_y;
	v[3] = (vCorners[2].Y - vCorners[1].Y) / (float)div_y;
	for (int i = 0; i <= div_y; ++i)
	{
		p[0] = vCorners[0].X + v[0] * i;
		p[1] = vCorners[0].Y + v[1] * i;
		p[2] = vCorners[1].X + v[2] * i;
		p[3] = vCorners[1].Y + v[3] * i;
		glBegin(GL_LINE_LOOP);
		glVertex2f(p[0], p[1]);
		glVertex2f(p[2], p[3]);
		glEnd();
	}
	//--

	//-- gizmo background
	layer->UpdateProjection(*this);
	vCorners = layer->vProjectedGizmo;
	glColor4f(0.0f, 0.8f, 0.0f, 0.2f);
	glBegin(GL_QUADS);
	glVertex2f(vCorners[0].X, vCorners[0].Y);
	glVertex2f(vCorners[1].X, vCorners[1].Y);
	glVertex2f(vCorners[2].X, vCorners[2].Y);
	glVertex2f(vCorners[3].X, vCorners[3].Y);
	glEnd();
	glColor4f(0.0f, 0.8f, 0.0f, 1.f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(vCorners[0].X, vCorners[0].Y);
	glVertex2f(vCorners[1].X, vCorners[1].Y);
	glVertex2f(vCorners[2].X, vCorners[2].Y);
	glVertex2f(vCorners[3].X, vCorners[3].Y);
	glEnd();
	//--

	//-- gizmo handles
	for (int i = 0; i < 4; ++i)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(vCorners[i].X - SHAPE_SIZE_RESIZE * this->zoom, vCorners[i].Y - SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(vCorners[i].X - SHAPE_SIZE_RESIZE * this->zoom, vCorners[i].Y + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(vCorners[i].X + SHAPE_SIZE_RESIZE * this->zoom, vCorners[i].Y + SHAPE_SIZE_RESIZE * this->zoom);
		glVertex2f(vCorners[i].X + SHAPE_SIZE_RESIZE * this->zoom, vCorners[i].Y - SHAPE_SIZE_RESIZE * this->zoom);
		glEnd();
	}
	//--

	glDisable(GL_BLEND);
}

