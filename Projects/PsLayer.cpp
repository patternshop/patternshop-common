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
#include "PsController.h"
#include "PsLayer.h"
#include "PsProject.h"

GLfloat Projection[16];
GLfloat ModelView[16];
GLfloat Viewport[4];

/*
** PsLayer utilise le constructeur par défaut d'PsShape, donc sans conteneur.
*/
PsLayer::PsLayer()
{
	ucData = NULL;
	iFinalTextureId = 0;
	iMaskTextureId = 0;
	fGizmoSize = 1000;
	fScale = 1.f;
}

/*
*/
PsLayer::~PsLayer()
{
	if (iFinalTextureId)
	{
		glDeleteTextures(1, &iFinalTextureId);
		iFinalTextureId = 0;
	}

	if (iMaskTextureId)
	{
		glDeleteTextures(1, &iMaskTextureId);
		iMaskTextureId = 0;
	}

	if (ucData)
	{
		delete[] ucData;
		ucData = 0;
	}
}

/*
** Charge les données d'une calque (ceci comprend toutes les images qu'elle contient) depuis un fichier.
*/
ErrID PsLayer::FileLoad(FILE* file)
{

	if (fread(&iWidth, sizeof(int), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&iHeight, sizeof(int), 1, file) != 1)
		return ERROR_FILE_READ;

	ucData = new uint8[iWidth * iHeight];
	if (fread(ucData, sizeof(*ucData), iWidth * iHeight, file) != iWidth * iHeight)
		return ERROR_FILE_READ;

	if (Register(iWidth, iHeight, ucData) != ERROR_NONE)
		return ERROR_FILE_READ;

	if (fread(&vTranslation, sizeof(PsVector), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&rRotation, sizeof(PsRotator), 1, file) != 1)
		return ERROR_FILE_READ;

	/*
	if (fread (&fPlaneWidth, sizeof (float), 1, file) != 1)
	return ERROR_FILE_READ;

	if (fread (&fPlaneHeight, sizeof (float), 1, file) != 1)
	return ERROR_FILE_READ;
	*/

	if (fread(&fScale, sizeof(float), 1, file) != 1)
		return ERROR_FILE_READ;

	return ERROR_NONE;
}

/*
** Sauvegarde toutes les données d'une calque dans un fichier.
*/
ErrID PsLayer::FileSave(FILE* file) const
{

	if (fwrite(&iWidth, sizeof(int), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&iHeight, sizeof(int), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(ucData, sizeof(*ucData), iWidth * iHeight, file) != iWidth * iHeight)
		return ERROR_FILE_WRITE;

	if (fwrite(&vTranslation, sizeof(PsVector), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&rRotation, sizeof(PsRotator), 1, file) != 1)
		return ERROR_FILE_WRITE;

	/*
	if (fwrite (&fPlaneWidth, sizeof (float), 1, file) != 1)
	return ERROR_FILE_WRITE;

	if (fwrite (&fPlaneHeight, sizeof (float), 1, file) != 1)
	return ERROR_FILE_WRITE;
	*/

	if (fwrite(&fScale, sizeof(float), 1, file) != 1)
		return ERROR_FILE_WRITE;

	return ERROR_NONE;
}

/*
** Crée l'instance d'object ainsi que la texture associée.
*/
ErrID	PsLayer::Register(int width, int height, uint8* channel)
{

	//-- copie des données
	this->ucData = channel;
	this->iWidth = width;
	this->iHeight = height;
	//--

	return CreateTexture();
}

/*
** Redimensionne la texture (Max 512x512)
*/
ErrID PsLayer::CreateTexture()
{
	//-- choix de la taille cible
	int h, w;
	int max_resol = 512;
	for (h = 1; h < iHeight && h < max_resol; )
		h <<= 1;
	for (w = 1; w < iWidth && w < max_resol; )
		w <<= 1;
	//--

	//-- préparation des données
	uint8* texture_buffer = new uint8[w * h];
	for (int x = w; x--; )
	{
		for (int y = h; y--; )
		{
			int v = 0, n = 0;
			int i = x * iWidth / w;
			do
			{
				int j = y * iHeight / h;
				do
				{
					v += ucData[i + j * iWidth];
					++n;
				} while (++j < (y + 1) * iHeight / h);
			} while (++i < (x + 1) * iWidth / w);
			texture_buffer[(x + y * w)] = (v / n > 127 ? 255 : 0);
		}
	}
	//--

	//-- creation de la texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &iMaskTextureId);
	glBindTexture(GL_TEXTURE_2D, iMaskTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);
	delete[] texture_buffer;
	//--

	return ERROR_NONE;
}

/* Perform one transform operation */
static void
Transform(GLfloat* matrix, GLfloat* in, GLfloat* out)
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

static void
DoTransform(GLfloat* in, GLfloat* out)
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
	gluLookAt(vEye.X, vEye.Y, vEye.Z, // oeuil   
		vEye.X, vEye.Y, 1.f, // point de fuite 
		0.0f, 1.0f, 0.0f); // vecteur d'orientation
	//glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
	glPopMatrix();
}

void PsLayer::UpdateProjection(PsRender& renderer)
{

	float fLayerWidth = renderer.doc_x;
	float fLayerHeight = renderer.doc_y;

	InitProjection(renderer);

	vProjected[0].X = -fLayerWidth / 2.f;
	vProjected[0].Y = -fLayerHeight / 2.f;
	vProjected[1].X = fLayerWidth / 2.f;
	vProjected[1].Y = -fLayerHeight / 2.f;
	vProjected[2].X = fLayerWidth / 2.f;
	vProjected[2].Y = fLayerHeight / 2.f;
	vProjected[3].X = -fLayerWidth / 2.f;
	vProjected[3].Y = fLayerHeight / 2.f;

	for (int i = 0; i < 4; ++i)
	{
		vProjected[i].Z = 0.f;

		// scale
		vProjected[i] = vProjected[i] * fScale;

		// rotation
		vProjected[i] = RotateVertex(vProjected[i], PsRotator::FromDegree(rRotation.Roll),
			PsRotator::FromDegree(rRotation.Pitch), PsRotator::FromDegree(rRotation.Yaw));

		// translation
		vProjected[i].X += vTranslation.X;
		vProjected[i].Y += vTranslation.Y;
		vProjected[i].Z += vTranslation.Z;

		// projection

		GLfloat quadV[4];
		quadV[0] = vProjected[i].X;
		quadV[1] = vProjected[i].Y;
		quadV[2] = vProjected[i].Z;
		quadV[3] = 1.f;

		GLfloat tmp[4];
		DoTransform(quadV, tmp);

		//glTexCoord4f(quadT[i][0] * tmp[3], quadT[i][1] * tmp[3], 0.0f, tmp[3]);
		//glVertex3fv(tmp);

		vProjected[i].X = tmp[0];
		vProjected[i].Y = tmp[1];
		vProjected[i].Z = tmp[2];

		fW[i] = tmp[3];

		// centrage de la perspective
		vProjected[i].X += renderer.doc_x / 2.f;
		vProjected[i].Y += renderer.doc_y / 2.f;

	}
}

void PsLayer::UpdateGizmoProjection(PsRender& renderer)
{

	float fLayerWidth = fGizmoSize;
	float fLayerHeight = renderer.doc_y;

	InitProjection(renderer);

	vProjectedGizmo[0].X = -fGizmoSize / 2.f;
	vProjectedGizmo[0].Y = -fGizmoSize / 2.f;
	vProjectedGizmo[1].X = fGizmoSize / 2.f;
	vProjectedGizmo[1].Y = -fGizmoSize / 2.f;
	vProjectedGizmo[2].X = fGizmoSize / 2.f;
	vProjectedGizmo[2].Y = fGizmoSize / 2.f;
	vProjectedGizmo[3].X = -fGizmoSize / 2.f;
	vProjectedGizmo[3].Y = fGizmoSize / 2.f;

	for (int i = 0; i < 4; ++i)
	{
		// lecture des positions des coins 
		vProjectedGizmo[i].Z = 0.f;

		// scale
		vProjectedGizmo[i] = vProjectedGizmo[i] * fScale;

		// rotation
		vProjectedGizmo[i] = RotateVertex(vProjectedGizmo[i], PsRotator::FromDegree(rRotation.Roll),
			PsRotator::FromDegree(rRotation.Pitch), PsRotator::FromDegree(rRotation.Yaw));

		// translation
		vProjectedGizmo[i].X += vTranslation.X;
		vProjectedGizmo[i].Y += vTranslation.Y;
		vProjectedGizmo[i].Z += vTranslation.Z;

		// projection    
		GLfloat quadV[4];
		quadV[0] = vProjectedGizmo[i].X;
		quadV[1] = vProjectedGizmo[i].Y;
		quadV[2] = vProjectedGizmo[i].Z;
		quadV[3] = 1.f;

		GLfloat tmp[4];
		DoTransform(quadV, tmp);

		vProjectedGizmo[i].X = tmp[0];
		vProjectedGizmo[i].Y = tmp[1];
		vProjectedGizmo[i].Z = 0.f;//tmp[2];
		fW[i] = 1.f;//tmp[3];

		// centrage de la perspective
		vProjectedGizmo[i].X += renderer.doc_x / 2.f;
		vProjectedGizmo[i].Y += renderer.doc_y / 2.f;

	}
}

bool PsLayer::MouseIsInside(int x, int y) const
{
	PsVector vPoint(x, y, 0);

	PsVector vCenter;
	for (int i = 0; i < 4; ++i)
		vCenter += vProjectedGizmo[i];
	vCenter /= 4;

	for (int i = 0; i < 4; ++i)
	{
		PsVector vN = vProjectedGizmo[(i + 1) % 4] - vProjectedGizmo[i];
		vN = RotateVertex(vN, 0, 0, 3.14f / 2.f);

		PsVector vAP = vPoint - vProjectedGizmo[i];
		PsVector vAC = vProjectedGizmo[(i + 2) % 4] - vProjectedGizmo[i];

		if (!SameSign(DotProduct2x2(vAP, vN), DotProduct2x2(vAC, vN)))
			return false;

	}

	return true;
}

/*
** Teste si un point est à l'un des emplacements de rotation; même remarques que pour
** le redimentionnement.
*/
bool PsLayer::InRotate(float px, float py, float zoom, int& i) const
{
	float	size = SHAPE_SIZE_ROTATE * zoom;

	for (i = 0; i < 4; ++i)
	{
		float angle = ToAngle(vProjectedGizmo[i].X, vProjectedGizmo[i].Y);
		float ax = size * cos(angle) - size * sin(angle);
		float ay = size * sin(angle) + size * cos(angle);

		if (px >= vProjectedGizmo[i].X + ax - size
			&& px <= vProjectedGizmo[i].X + ax + size
			&& py >= vProjectedGizmo[i].Y + ay - size
			&& py <= vProjectedGizmo[i].Y + ay + size)
			return true;
	}

	return false;
}

/*
** Retourne l'angle entre le centre de l'PsShape et le point "ax, ay", en radians.
*/
float	PsLayer::ToAngle(float ax, float ay) const
{
	float tx = vTranslation.X;
	float ty = vTranslation.Y;

	if (ax < tx)
		return (float)atan((ay - ty) / (ax - tx)) + PS_MATH_PI;
	else if (ax > tx)
		return (float)atan((ay - ty) / (ax - tx));
	else
		return (ay < ty ? -PS_MATH_PI : PS_MATH_PI) / 2.0f;
}

/*
** Teste si un point est sur l'une des poignées de redimentionnement d'une PsLayer, et enregistre
** dans "i" l'index de la poignée (0 = haut/gauche, etc dans le sens horaire). Cette fonction doit
** prendre le zoom en paramètre, pour que la taille des poignées ne soient pas affectées par lui
** (on doit donc pouvoir annuler ses effets).
*/
bool PsLayer::InResize(float px, float py, float zoom, int& i) const
{
	float	size = SHAPE_SIZE_RESIZE * zoom;

	for (i = 0; i < 4; ++i)
		if (px >= vProjectedGizmo[i].X - size
			&& px <= vProjectedGizmo[i].X + size
			&& py >= vProjectedGizmo[i].Y - size
			&& py <= vProjectedGizmo[i].Y + size)
			return true;

	return false;
}