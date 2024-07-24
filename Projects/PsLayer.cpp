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
#include "PsTexture.h"
#include "PsLayer.h"
#include "PsProject.h"

GLfloat Projection[16];
GLfloat ModelView[16];
GLfloat Viewport[4];

/*
** PsLayer uses the default constructor of PsShape, so without container.
*/
PsLayer::PsLayer()
{
	this->ucData = NULL;
	this->iFinalTextureId = 0;
	this->iMaskTextureId = 0;
	this->fGizmoSize = 1000;
	this->fScale = 1.f;
}

/*
*/
PsLayer::~PsLayer()
{
	if (this->iFinalTextureId)
	{
		glDeleteTextures(1, &this->iFinalTextureId);
		this->iFinalTextureId = 0;
	}

	if (this->iMaskTextureId)
	{
		glDeleteTextures(1, &this->iMaskTextureId);
		this->iMaskTextureId = 0;
	}

	if (this->ucData)
	{
		delete[] this->ucData;
		this->ucData = 0;
	}
}

/*
** Creates the object instance as well as the associated texture.
*/
ErrID PsLayer::Register(int width, int height, uint8* channel)
{
	//-- copy data
	this->ucData = channel;
	this->iWidth = width;
	this->iHeight = height;
	//--

	return this->CreateTexture();
}

/*
** Resizes the texture (Max 512x512)
*/
ErrID PsLayer::CreateTexture()
{
	//-- choose target size
	int h, w;
	int max_resol = 512;
	for (h = 1; h < this->iHeight && h < max_resol; )
		h <<= 1;
	for (w = 1; w < this->iWidth && w < max_resol; )
		w <<= 1;
	//--

	//-- prepare data
	uint8* texture_buffer = new uint8[w * h];
	for (int x = w; x--; )
	{
		for (int y = h; y--; )
		{
			int v = 0, n = 0;
			int i = x * this->iWidth / w;
			do
			{
				int j = y * this->iHeight / h;
				do
				{
					v += this->ucData[i + j * this->iWidth];
					++n;
				} while (++j < (y + 1) * this->iHeight / h);
			} while (++i < (x + 1) * this->iWidth / w);
			texture_buffer[(x + y * w)] = (v / n > 127 ? 255 : 0);
		}
	}
	//--

	//-- create texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &this->iMaskTextureId);
	glBindTexture(GL_TEXTURE_2D, this->iMaskTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);
	delete[] texture_buffer;
	//--

	return ERROR_NONE;
}

/* Perform one transform operation */
static void Transform(GLfloat* matrix, GLfloat* in, GLfloat* out)
{
	int ii;

	for (ii = 0; ii < 4; ++ii) {
		out[ii] =
			in[0] * matrix[0 * 4 + ii] +
			in[1] * matrix[1 * 4 + ii] +
			in[2] * matrix[2 * 4 + ii] +
			in[3] * matrix[3 * 4 + ii];
	}
}

/* Transform a vertex from object coordinates to window coordinates.
* Lighting is left as an exercise for the reader.
*/

static void DoTransform(GLfloat* in, GLfloat* out)
{
	GLfloat tmp[4];
	GLfloat invW;       /* 1/w */

	/* Modelview xform */
	Transform(ModelView, in, tmp);

	/* Lighting calculation goes here! */

	/* Projection xform */
	Transform(Projection, tmp, out);

	if (out[3] == 0.0f) /* do what? */
		return;

	invW = 1.0f / out[3];

	// Perspective divide
	out[0] *= invW;
	out[1] *= invW;
	out[2] *= invW;
	// Map to 0..1 range 
	/*
	out[0] = out[0] * 0.5f + 0.5f;
	out[1] = out[1] * 0.5f + 0.5f;
	out[2] = out[2] * 0.5f + 0.5f;
	*/

	// Map to viewport 
	out[0] = out[0] * Viewport[2] + Viewport[0];
	out[1] = out[1] * Viewport[3] + Viewport[1];

	// Store inverted w for performance
	out[3] = invW;
}

void PsLayer::InitProjection(PsRender& renderer)
{
	/* Retrieve the view port */
	Viewport[0] = (GLfloat)0;
	Viewport[1] = (GLfloat)0;
	Viewport[2] = (GLfloat)renderer.doc_x;
	Viewport[3] = (GLfloat)renderer.doc_y;

	/* Retrieve the projection matrix */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//glFrustum(-10, 10, -10, 10, 1.f, 20001.f); 
	gluPerspective(45.0f, Viewport[2] / Viewport[3], 10000.f, 20000.f);
	glGetFloatv(GL_PROJECTION_MATRIX, Projection);
	glPopMatrix();

	/* Retrieve the model view matrix */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//glTranslatef(renderer.size_x / 2.f, 0.f, -10001.f);
	PsVector vEye = renderer.GetEyeLocation();
	gluLookAt(vEye.X, vEye.Y, vEye.Z, // eye   
		vEye.X, vEye.Y, 1.f, // vanishing point 
		0.0f, 1.0f, 0.0f); // orientation vector
	//glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
	glPopMatrix();
}

void PsLayer::UpdateProjection(PsRender& renderer)
{
	float fLayerWidth = renderer.doc_x;
	float fLayerHeight = renderer.doc_y;

	this->InitProjection(renderer);

	this->vProjected[0].X = -fLayerWidth / 2.f;
	this->vProjected[0].Y = -fLayerHeight / 2.f;
	this->vProjected[1].X = fLayerWidth / 2.f;
	this->vProjected[1].Y = -fLayerHeight / 2.f;
	this->vProjected[2].X = fLayerWidth / 2.f;
	this->vProjected[2].Y = fLayerHeight / 2.f;
	this->vProjected[3].X = -fLayerWidth / 2.f;
	this->vProjected[3].Y = fLayerHeight / 2.f;

	for (int i = 0; i < 4; ++i)
	{
		this->vProjected[i].Z = 0.f;

		// scale
		this->vProjected[i] = this->vProjected[i] * this->fScale;

		// rotation
		this->vProjected[i] = RotateVertex(this->vProjected[i], PsRotator::FromDegree(this->rRotation.Roll),
			PsRotator::FromDegree(this->rRotation.Pitch), PsRotator::FromDegree(this->rRotation.Yaw));

		// translation
		this->vProjected[i].X += this->vTranslation.X;
		this->vProjected[i].Y += this->vTranslation.Y;
		this->vProjected[i].Z += this->vTranslation.Z;

		// projection

		GLfloat quadV[4];
		quadV[0] = this->vProjected[i].X;
		quadV[1] = this->vProjected[i].Y;
		quadV[2] = this->vProjected[i].Z;
		quadV[3] = 1.f;

		GLfloat tmp[4];
		DoTransform(quadV, tmp);

		//glTexCoord4f(quadT[i][0] * tmp[3], quadT[i][1] * tmp[3], 0.0f, tmp[3]);
		//glVertex3fv(tmp);

		this->vProjected[i].X = tmp[0];
		this->vProjected[i].Y = tmp[1];
		this->vProjected[i].Z = tmp[2];

		this->fW[i] = tmp[3];

		// center the perspective
		this->vProjected[i].X += renderer.doc_x / 2.f;
		this->vProjected[i].Y += renderer.doc_y / 2.f;
	}
}

void PsLayer::UpdateGizmoProjection(PsRender& renderer)
{
	float fLayerWidth = this->fGizmoSize;
	float fLayerHeight = renderer.doc_y;

	this->InitProjection(renderer);

	this->vProjectedGizmo[0].X = -this->fGizmoSize / 2.f;
	this->vProjectedGizmo[0].Y = -this->fGizmoSize / 2.f;
	this->vProjectedGizmo[1].X = this->fGizmoSize / 2.f;
	this->vProjectedGizmo[1].Y = -this->fGizmoSize / 2.f;
	this->vProjectedGizmo[2].X = this->fGizmoSize / 2.f;
	this->vProjectedGizmo[2].Y = this->fGizmoSize / 2.f;
	this->vProjectedGizmo[3].X = -this->fGizmoSize / 2.f;
	this->vProjectedGizmo[3].Y = this->fGizmoSize / 2.f;

	for (int i = 0; i < 4; ++i)
	{
		// reading the positions of the corners
		this->vProjectedGizmo[i].Z = 0.f;

		// scale
		this->vProjectedGizmo[i] = this->vProjectedGizmo[i] * this->fScale;

		// rotation
		this->vProjectedGizmo[i] = RotateVertex(this->vProjectedGizmo[i], PsRotator::FromDegree(this->rRotation.Roll),
			PsRotator::FromDegree(this->rRotation.Pitch), PsRotator::FromDegree(this->rRotation.Yaw));

		// translation
		this->vProjectedGizmo[i].X += this->vTranslation.X;
		this->vProjectedGizmo[i].Y += this->vTranslation.Y;
		this->vProjectedGizmo[i].Z += this->vTranslation.Z;

		// projection    
		GLfloat quadV[4];
		quadV[0] = this->vProjectedGizmo[i].X;
		quadV[1] = this->vProjectedGizmo[i].Y;
		quadV[2] = this->vProjectedGizmo[i].Z;
		quadV[3] = 1.f;

		GLfloat tmp[4];
		DoTransform(quadV, tmp);

		this->vProjectedGizmo[i].X = tmp[0];
		this->vProjectedGizmo[i].Y = tmp[1];
		this->vProjectedGizmo[i].Z = 0.f; // tmp[2];
		this->fW[i] = 1.f; // tmp[3];

		// centering the perspective
		this->vProjectedGizmo[i].X += renderer.doc_x / 2.f;
		this->vProjectedGizmo[i].Y += renderer.doc_y / 2.f;
	}
}

bool PsLayer::MouseIsInside(int x, int y) const
{
	PsVector vPoint(x, y, 0);

	PsVector vCenter;
	for (int i = 0; i < 4; ++i)
		vCenter += this->vProjectedGizmo[i];
	vCenter /= 4;

	for (int i = 0; i < 4; ++i)
	{
		PsVector vN = this->vProjectedGizmo[(i + 1) % 4] - this->vProjectedGizmo[i];
		vN = RotateVertex(vN, 0, 0, 3.14f / 2.f);

		PsVector vAP = vPoint - this->vProjectedGizmo[i];
		PsVector vAC = this->vProjectedGizmo[(i + 2) % 4] - this->vProjectedGizmo[i];

		if (!SameSign(DotProduct2x2(vAP, vN), DotProduct2x2(vAC, vN)))
			return false;
	}

	return true;
}

/*
** Tests if a point is at one of the rotation locations; same remarks as for resizing.
*/
bool PsLayer::InRotate(float px, float py, float zoom, int& i) const
{
	float size = SHAPE_SIZE_ROTATE * zoom;

	for (i = 0; i < 4; ++i)
	{
		float angle = this->ToAngle(this->vProjectedGizmo[i].X, this->vProjectedGizmo[i].Y);
		float ax = size * cos(angle) - size * sin(angle);
		float ay = size * sin(angle) + size * cos(angle);

		if (px >= this->vProjectedGizmo[i].X + ax - size
			&& px <= this->vProjectedGizmo[i].X + ax + size
			&& py >= this->vProjectedGizmo[i].Y + ay - size
			&& py <= this->vProjectedGizmo[i].Y + ay + size)
			return true;
	}

	return false;
}

/*
** Returns the angle between the center of the PsShape and the point "ax, ay", in radians.
*/
float PsLayer::ToAngle(float ax, float ay) const
{
	float tx = this->vTranslation.X;
	float ty = this->vTranslation.Y;

	if (ax < tx)
		return (float)atan((ay - ty) / (ax - tx)) + PS_MATH_PI;
	else if (ax > tx)
		return (float)atan((ay - ty) / (ax - tx));
	else
		return (ay < ty ? -PS_MATH_PI : PS_MATH_PI) / 2.0f;
}

/*
** Tests if a point is on one of the resizing handles of a PsLayer, and records
** in "i" the index of the handle (0 = top/left, etc. clockwise). This function must
** take the zoom as a parameter, so that the size of the handles is not affected by it
** (we must therefore be able to cancel its effects).
*/
bool PsLayer::InResize(float px, float py, float zoom, int& i) const
{
	float size = SHAPE_SIZE_RESIZE * zoom;

	for (i = 0; i < 4; ++i)
		if (px >= this->vProjectedGizmo[i].X - size
			&& px <= this->vProjectedGizmo[i].X + size
			&& py >= this->vProjectedGizmo[i].Y - size
			&& py <= this->vProjectedGizmo[i].Y + size)
			return true;

	return false;
}
