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
#include <stdio.h>
#include "PsProject.h"
#include "PsController.h"
#include "PsAction.h"
#include "PsMatrix.h"
#include "PsPattern.h"
#include "PsSoftRender.h"
#include "PsWinPropertiesCx.h"

// project parameters
#define ZOOM_COEF		0.02f
#define LOG_SIZE		50

// File format version

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define CANDIDATE_VERSION 2

//--------------------------AUTO--------------------------

#define PROJECT_VERSION	(MAJOR_VERSION * 1000000 + MINOR_VERSION * 1000 + CANDIDATE_VERSION)

#define BIG_ENDIAN_FILE 1
#define LITTLE_ENDIAN_FILE 0

#ifdef __BIG_ENDIAN__
#define ACCEPTED_ENDIAN_FILE BIG_ENDIAN_FILE
#else
#define ACCEPTED_ENDIAN_FILE LITTLE_ENDIAN_FILE
#endif
//--------------------------AUTO (END)----------------------

/*
** Initialement, aucun patron n'est selectionné, ni aucune matrice, ni aucune image, etc.
*/
PsProject::PsProject() :
	pattern(0),
	matrix(0),
	image(0),
	shape(0),
	center(true),
	log_insert(false)
{
	iColor[0] = 255;
	iColor[1] = 255;
	iColor[2] = 255;
	bHideColor = false;
	bNeedSave = false;
	iLayerId = 0;
	bPatternsIsSelected = false;
}


PsProject::~PsProject()
{
	LogFlush();
}

/*
** Duplique l'image actuellement sélectionnée
*/
ErrID				PsProject::CloneImage()
{
	uint8* buffer1;
	uint8* buffer2;
	PsImage* image;
	size_t		size;

	if (!this->image)
		return ERROR_IMAGE_SELECT;

	image = new PsImage(this->matrix);

	buffer1 = this->image->GetTexture().GetBuffer(size);
	buffer2 = new uint8[size];
	memcpy(buffer2, buffer1, size * sizeof(*buffer2));

	if (!image->TextureFromBuffer(buffer2))
		return ERROR_IMAGE_LOAD;

	image->SetAngle(this->image->GetAngle());
	image->SetSize(this->image->w, this->image->h);
	image->SetTorsion(this->image->i, this->image->j);

	if (this->matrix)
	{
		this->matrix->images.push_back(image);
		image->x = 0;
		image->y = 0;
	}
	else
	{
		this->images.push_back(image);
		image->SetPosition(PROJECT_VERSION / 2.0f, GetHeight() / 2.0f);
	}

	this->LogAdd(new LogNewImage(*this, image, true));
	this->SelectImage(image);

	return ERROR_NONE;
}

/*
** Duplique la matrice actuellement sélectionnée
*/
ErrID					PsProject::CloneMatrix()
{
	uint8* buffer1;
	uint8* buffer2;
	PsMatrix* matrix;
	ImageList::iterator		ti;
	PsImage* image;
	size_t				size;

	if (!this->matrix)
		return ERROR_MATRIX_SELECT;

	matrix = new PsMatrix();

	this->matrices.push_back(matrix);

	matrix->SetAngle(this->matrix->GetAngle());
	matrix->SetSize(this->matrix->w, this->matrix->h);
	matrix->SetTorsion(this->matrix->i, this->matrix->j);
	matrix->SetPosition(GetWidth() / 2.0f, GetHeight() / 2.0f);

	this->LogAdd(new LogNewMatrix(*this, matrix, true));

	for (ti = this->matrix->images.begin(); ti != this->matrix->images.end(); ++ti)
	{
		image = new PsImage(matrix);

		buffer1 = (*ti)->GetTexture().GetBuffer(size);
		buffer2 = new uint8[size];
		memcpy(buffer2, buffer1, size * sizeof(*buffer2));

		if (!image->TextureFromBuffer(buffer2))
			return ERROR_IMAGE_LOAD;

		matrix->images.push_back(image);
		image->SetAngle((*ti)->GetAngle());
		image->SetSize((*ti)->w, (*ti)->h);
		image->SetTorsion((*ti)->i, (*ti)->j);
		image->x = (*ti)->x;
		image->y = (*ti)->y;

		this->LogAdd(new LogNewImage(*this, image, true));
	}

	this->SelectMatrix(matrix);

	return ERROR_NONE;
}

/*
** Remplace l'image actuellement sélectionnée
*/
ErrID	PsProject::ReplaceImage(const char* file)
{
	if (!this->image)
		return ERROR_IMAGE_SELECT;

	this->LogAdd(new LogReplace(*this, this->image));

	if (!this->image->TextureFromFile(file, false))
		return ERROR_IMAGE_LOAD;

	return ERROR_NONE;
}

/*
** Supprime l'image sélectionnée de la matrice qui la contient.
*/
ErrID	PsProject::DelImage()
{
	ImageList::iterator		ti;
	MatrixList::iterator	tm;

	if (!image)
		return ERROR_IMAGE_SELECT;

	this->LogAdd(new LogDelImage(*this, image, false));

	if (!image->parent)
		this->images.remove(image);
	else
		this->matrix->images.remove(image);

	delete this->image;

	PsMatrix* b = matrix;
	SelectImage(0);
	matrix = b;

	return ERROR_NONE;
}

/*
** Supprime la matrice sélectionnée, et en sélectionne une autre si il en reste.
*/
ErrID					PsProject::DelMatrix()
{
	ImageList::iterator	t;

	if (!this->matrix)
		return ERROR_MATRIX_SELECT;

	for (t = this->matrix->images.begin(); t != this->matrix->images.end(); ++t)
		this->LogAdd(new LogDelImage(*this, *t, false));

	this->LogAdd(new LogDelMatrix(*this, this->matrix, false));

	this->matrices.remove(this->matrix);
	delete this->matrix;

	SelectMatrix(0);

	return ERROR_NONE;
}

/*
** Supprime le patron sélectionné, et en sélectionne un autre si il en reste.
*/
ErrID	PsProject::DelPattern()
{
	if (!pattern)
		return ERROR_PATTERN_SELECT;

	delete pattern;
	pattern = 0;

	return ERROR_NONE;
}

/*
** Importe une nouvelle image hors matrice.
*/
ErrID		PsProject::NewImage(const char* file)
{
	PsImage* image;

	image = new PsImage(NULL);

	if (!image->TextureFromFile(file))
		return ERROR_IMAGE_LOAD;

	image->SetPosition(GetWidth() / 2.0f, GetHeight() / 2.0f);
	images.push_back(image);

	SelectImage(image);

	this->LogAdd(new LogNewImage(*this, image, true));

	return ERROR_NONE;
}

/*
** Importe une nouvelle image dans la matrice courante, si il y en a une.
*/
ErrID		PsProject::NewMotif(const char* file)
{
	PsImage* image;
	float	x;
	float	y;

	if (!matrix)
	{
		if (this->matrices.size() == 0)
			this->NewMatrix();

		matrix = *this->matrices.begin();
	}

	image = new PsImage(matrix);

	if (!image->TextureFromFile(file))
		return ERROR_IMAGE_LOAD;

	matrix->images.push_back(image);
	matrix->GetPosition(x, y);
	image->SetPosition(x, y);

	SelectImage(image);

	this->LogAdd(new LogNewImage(*this, image, true));

	return ERROR_NONE;
}

/*
** Crée une nouvelle matrice dans le document. Sa taille initiale est pour l'instant le quart
** de celle du document, mais ceci est totalement arbitraire(oui le quart et non pas le 8ième,
** la remarque de PsImage::TextureFromBuffer s'applique ici également).
*/
ErrID	PsProject::NewMatrix()
{
	matrix = new PsMatrix();
	image = 0;

	matrices.push_back(matrix);
	matrix->SetPosition(GetWidth() / 2.0f, GetHeight() / 2.0f);
	matrix->SetSize((float)PsMatrix::default_w, (float)PsMatrix::default_h);

	SelectMatrix(matrix);

	this->LogAdd(new LogNewMatrix(*this, matrix, true));

	return ERROR_NONE;
}

/*
** Crée un nouveau patron dans le document.
*/
ErrID	PsProject::NewPattern(const char* file)
{
	if (pattern)
		delete pattern;

	pattern = new PsPattern();

	if (!pattern->TextureFromFile(file))
	{
		delete pattern;
		pattern = 0;
		return ERROR_PATTERN_LOAD;
	}

	//-- paramétrage initial
	for (int i = 0; i < pattern->GetChannelsCount(); ++i)
	{
		PsLayer* layer = pattern->aLayers[i];
		layer->vTranslation.X = GetWidth() / 2.f;
		layer->vTranslation.Y = GetHeight() / 2.f;
	}
	//--

	pattern->UpdateScale(GetWidth(), GetHeight());

	this->pattern = pattern;
	iLayerId = 0;

	return ERROR_NONE;
}

/*
** Charge le projet depuis un fichier. La macro constante "PROJECT_VERSION" permet de vérifier la version
** de la sauvegarde et sert donc de magic number, le reste devrait être assez explicite, on charge tous
** les champs du projet en s'aidant de leurs methodes FileLoad.
*/

#define PROJECT_VERSION_OLD_BETA 3213654 // 111222333

ErrID			PsProject::FileLoad(const char* path)
{
	PsMatrix* matrix;
	FILE* file;
	ErrID		err;
	size_t		n;
	int			x;
	int			y;

	if (!(file = fopen(path, "rb")))
		return ERROR_FILE_ACCESS;

	PsController::Instance().SetProgress(-1);

	LogFlush();

	fread(&n, sizeof(size_t), 1, file);

	if (n != ACCEPTED_ENDIAN_FILE)
	{
		PsController::Instance().SetProgress(-2);
		return ERROR_FILE_VERSION;
	}

	fread(&n, sizeof(size_t), 1, file);

	if (n != PROJECT_VERSION && n != PROJECT_VERSION_OLD_BETA) // FIXME
	{
		PsController::Instance().SetProgress(-2);
		return ERROR_FILE_VERSION;
	}

	fread(&x, sizeof(x), 1, file);
	fread(&y, sizeof(y), 1, file);
	renderer.SetDocSize(x, y);

	PsController::Instance().SetProgress(10);

	if (n != PROJECT_VERSION_OLD_BETA) // FIXME
	{
		if (fread(&n, sizeof(n), 1, file) != 1)
		{
			PsController::Instance().SetProgress(-2);
			return ERROR_FILE_READ;
		}
		while (n--)
		{
			images.push_back((image = new PsImage(NULL)));

			if ((err = image->FileLoad(file)) != ERROR_NONE)
			{
				PsController::Instance().SetProgress(-2);
				return err;
			}
		}
	}

	fread(&n, sizeof(n), 1, file);

	for (uint32 i = 0; i < n; ++i)
	{
		PsController::Instance().SetProgress(10 + 90 * i / n);

		matrices.push_back((matrix = new PsMatrix()));

		if ((err = matrix->FileLoad(file)) != ERROR_NONE)
		{
			PsController::Instance().SetProgress(-2);
			return err;
		}
	}

	bool pattern_exist = false;
	fread(&pattern_exist, sizeof(pattern_exist), 1, file);
	if (pattern_exist)
	{
		pattern = new PsPattern();
		if ((err = pattern->FileLoad(file)) != ERROR_NONE)
		{
			PsController::Instance().SetProgress(-2);
			return err;
		}
		pattern->UpdateScale(GetWidth(), GetHeight());
	}

	fread(&bHideColor, sizeof(bHideColor), 1, file);
	fread(&iColor, sizeof(iColor), 1, file);

	fclose(file);

	center = true;

	PsController::Instance().SetProgress(-2);

	return ERROR_NONE;
}

/*
** Sauvegarde un projet dans un fichier(voir FileLoad).
*/
ErrID			PsProject::FileSave(const char* path)
{
	FILE* file;
	ErrID		err;
	size_t	n;
	int		x;
	int		y;

	if (!(file = fopen(path, "wb")))
		return ERROR_FILE_ACCESS;

	n = ACCEPTED_ENDIAN_FILE;
	fwrite(&n, sizeof(size_t), 1, file);

	n = PROJECT_VERSION;
	fwrite(&n, sizeof(size_t), 1, file);

	renderer.GetDocSize(x, y);
	fwrite(&x, sizeof(int), 1, file);
	fwrite(&y, sizeof(int), 1, file);

	n = images.size();

	if (fwrite(&n, sizeof(size_t), 1, file) != 1)
		return ERROR_FILE_WRITE;

	for (ImageList::const_iterator i = images.begin(); i != images.end(); ++i)
		if ((err = (*i)->FileSave(file)) != ERROR_NONE)
			return err;

	n = matrices.size();
	fwrite(&n, sizeof(size_t), 1, file);

	for (MatrixList::const_iterator i = matrices.begin(); i != matrices.end(); ++i)
		if ((err = (*i)->FileSave(file)) != ERROR_NONE)
		{
			fclose(file);

			return err;
		}

	bool pattern_exist = false;
	if (pattern) pattern_exist = true;
	fwrite(&pattern_exist, sizeof(pattern_exist), 1, file);
	if (pattern_exist)
	{
		if ((err = pattern->FileSave(file)) != ERROR_NONE)
		{
			fclose(file);
			return err;
		}
	}

	fwrite(&bHideColor, sizeof(bHideColor), 1, file);
	fwrite(&iColor, sizeof(iColor), 1, file);

	fclose(file);

	this->bNeedSave = false;

	return ERROR_NONE;
}

/*
** Remet un projet à zéro, en effacant tout ce qu'il contient. À terme, cette fonction
** pourait devenir inutile, et être recopiée simplement dans le destructeur de "PsProject".
*/
void						PsProject::LogFlush()
{
	LogList::iterator		t;
	MatrixList::iterator	i;

	for (t = this->log_redo.begin(); t != this->log_redo.end(); ++t)
		delete* t;

	for (t = this->log_undo.begin(); t != this->log_undo.end(); ++t)
		delete* t;

	for (i = matrices.begin(); i != matrices.end(); ++i)
		delete* i;

	matrices.clear();

	if (pattern)
	{
		delete pattern;
		pattern = 0;
	}

	matrix = 0;
	image = 0;
	shape = 0;
}

/*
** Insere un nouvel élement dans le log
*/
void					PsProject::LogAdd(PsAction* log)
{
	LogList::iterator	t;

	for (t = this->log_redo.begin(); t != this->log_redo.end(); ++t)
		delete* t;

	this->log_redo.clear();

	if (log)
		this->log_undo.push_back(log);

	while (this->log_undo.size() > LOG_SIZE)
	{
		delete this->log_undo.front();
		this->log_undo.pop_front();
	}

	this->log_insert = false;
	this->bNeedSave = true;
}

/*
** Vérifie si une action peut être rétablie
*/
bool	PsProject::LogCanRedo() const
{
	return !this->log_redo.empty();
}

/*
** Vérifie si une action peut être annulée
*/
bool	PsProject::LogCanUndo() const
{
	return !this->log_undo.empty();
}

/*
** Initialise une nouvelle action(marque la fin de l'action précedente)
*/
void	PsProject::LogInit()
{
	this->log_insert = true;
}

/*
** Retourne le nombre d'éléments dans la liste d'annulation
*/
int	PsProject::LogUndoCount()
{
	return this->log_undo.size();
}

/*
** Retourne le nom de la dernière action "refaisable" enregistrée
*/
const char* PsProject::LogRedoLastName() const
{
	return(*this->log_redo.rbegin())->Name();
}

/*
** Retourne le nom de la dernière action "annulable" enregistrée
*/
const char* PsProject::LogUndoLastName() const
{
	return(*this->log_undo.rbegin())->Name();
}

/*
** Vérifie si l'action est nouvelle, et doit être enregistrée(et reset le flag le cas échéant)
*/
bool	PsProject::LogMustAdd() const
{
	return this->log_insert;
}

/*
** Reproduit la prochaine action
*/
void	PsProject::LogRedo()
{
	PsAction* redo;
	PsAction* undo;

	if (!this->log_redo.empty())
	{
		redo = *this->log_redo.rbegin();
		this->log_redo.pop_back();
		undo = redo->Execute();

		if (undo)
		{
			this->log_undo.push_back(undo);
			this->bNeedSave = true;
		}

		while (this->log_undo.size() > LOG_SIZE)
		{
			delete this->log_undo.front();
			this->log_undo.pop_front();
		}

		delete redo;
	}
}

/*
** Annule la dernière action
*/
void		PsProject::LogUndo()
{
	PsAction* redo;
	PsAction* undo;

	if (!this->log_undo.empty())
	{
		undo = *this->log_undo.rbegin();
		this->log_undo.pop_back();
		redo = undo->Execute();

		this->bNeedSave = true;

		if (redo)
			this->log_redo.push_back(redo);

		while (this->log_redo.size() > LOG_SIZE)
		{
			delete this->log_redo.front();
			this->log_redo.pop_front();
		}

		delete undo;
	}
}

/*
** Change la couleur de la matrice active, si il y en a une.
*/
ErrID	PsProject::MatrixColor()
{
	if (!matrix)
		return ERROR_MATRIX_SELECT;

	matrix->DoChangeColor();

	return ERROR_NONE;
}

/*
** Annule les transformations de la matrice active, si il y en a une.
*/
ErrID	PsProject::MatrixReset()
{
	if (!matrix)
		return ERROR_MATRIX_SELECT;

	this->LogAdd(new LogResize(*this, this->matrix, this->matrix->x, this->matrix->y, this->matrix->w, this->matrix->h, false));
	this->LogAdd(new LogRotate(*this, this->matrix, this->matrix->r, false));
	this->LogAdd(new LogTorsio(*this, this->matrix, this->matrix->i, this->matrix->j));

	matrix->DoResetAll();

	return ERROR_NONE;
}

/*
** Déclanche l'affichage du projet dans le contexte actif, en utilisant un rendu
** hardware(donc OpenGL).
*/
void	PsProject::RenderToScreen()
{
	int	x;
	int	y;

	if (center)
	{
		renderer.CenterView();
		center = false;
	}

	renderer.GetSize(x, y);
	renderer.SetEngine(PsRender::ENGINE_HARDWARE);
	renderer.Render(*this, x, y);

#ifdef _WINDOWS
	int m_glErrorCode = glGetError();
	if (m_glErrorCode != GL_NO_ERROR)
	{
		const GLubyte* estring;
		CString mexstr;
		estring = gluErrorString(m_glErrorCode);
		mexstr.Format("RenderToScreen:\n\tAn OpenGL error occurred: %s\n", estring);
		AfxMessageBox(mexstr, MB_OK | MB_ICONEXCLAMATION);
		TRACE0(mexstr);
	}
#endif /* _WINDOWS */
}

/*
** Déclanche le rendu dans un fichier
*/
bool	PsProject::RenderToFile(const char* filename, int x, int y)
{
	PsController::Instance().SetProgress(-1);

	renderer.SetZone((float)GetWidth(), (float)GetHeight());

	InitSoftwareFile(x, y);

	PsController::Instance().SetProgress(5);

	renderer.SetEngine(PsRender::ENGINE_SOFTWARE);
	renderer.Render(*this, x, y);
	renderer.Recalc();

	PsController::Instance().SetProgress(90);

	flushSoftwareFile(filename, bHideColor);

	PsController::Instance().SetProgress(-2);

	return true;
}

/*
** Sélectionne une image dans le projet(et la matrice qui la contient).
*/
void	PsProject::SelectImage(PsImage* image)
{
	this->matrix = image ? image->GetParent() : 0;
	this->image = image;
	this->shape = image;

	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogProject();

	if (dlgPropreties)
	{
		//dlgPropreties->FocusImageInformation();
		dlgPropreties->UpdateInformation(this);
	}
}

/*
** Sélectionne une matrice dans le projet.
*/
void	PsProject::SelectMatrix(PsMatrix* matrix)
{
	this->matrix = matrix;
	this->image = 0;
	this->shape = matrix;

	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogProject();

	if (dlgPropreties)
	{
		//dlgPropreties->FocusMatrixInformation();
		dlgPropreties->UpdateInformation(this);
	}
}

/*
** Sélectionne un patron. Seul le patron sélectionné est affiché, et si aucun patron n'est
** sélectionné(SelectPattern(0)), alors on voit simplement le fond, comme à la création
** d'un nouveau projet.
*/
/*void	PsProject::SelectPattern(PsPattern* pattern)
{
	this->pattern = pattern;
}*/

/*
** Déplacement de l'outil "loupe"(magnify).
*/
void		PsProject::ToolMagnifyDrag(int y, int old_x, int old_y)
{
	float	zoom = prev_zoom + (y - old_y) * ZOOM_COEF;

	if (y > old_y)
		PsController::Instance().SetCursor(CURSOR_MAGNIFY2);
	else
		PsController::Instance().SetCursor(CURSOR_MAGNIFY3);

	/* static float fx, fy, ox, oy;
	if (prev_zoom == renderer.zoom)
	{
		renderer.Convert(old_x, old_y, fx, fy);
		ox = renderer.scroll_x;
		oy = renderer.scroll_y;
	} */

	renderer.SetZoom(zoom);

	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogOverview();
}

/*
** Initialisation de l'outil "loupe".
*/
PsController::Tool	PsProject::ToolMagnifyStart()
{
	prev_zoom = renderer.zoom;

	return PsController::TOOL_MAGNIFY_ZOOM;
}

/*
** Déplacement de l'outil "Modify"(qui a besoin de savoir dans quel mode il est, pour
** connaitre l'operation à effectuer parmis déplacement, rotation, redimentionnement...)
*/
void		PsProject::ToolModifyMove(int x, int y, int old_x, int old_y, PsController::Tool tool)
{
	float	fx;
	float	fy;
	float	sx;
	float	sy;
	float	tx;
	float	ty;
	bool	constrain = PsController::Instance().GetOption(PsController::OPTION_CONSTRAIN);
	bool	reflect = PsController::Instance().GetOption(PsController::OPTION_REFLECT);


	renderer.Convert(x, y, fx, fy);
	renderer.Convert(old_x, old_y, sx, sy);

	if (bPatternsIsSelected && !pattern->hide)
	{
		PsLayer* layer = pattern->aLayers[iLayerId];

		switch (tool)
		{
		case PsController::TOOL_MODIFY_ROTATE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogPatternRotate(*this));

			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));
			layer->rRotation.Yaw = prev_r + PsRotator::ToDegree(-(layer->ToAngle(fx, fy) - layer->ToAngle(sx, sy)));
			break;
		}

		case PsController::TOOL_MODIFY_MOVE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogPatternTranslate(*this));

			PsVector vTranslation(fx - sx, fy - sy, 0.f);

			PsLayer* layer = pattern->aLayers[iLayerId];

			PsVector vEye = renderer.GetEyeLocation();

			float fDistance = (prev_origin - vEye).Size();
			float fDistanceZ0 = (prev_origin_z0 - vEye).Size();

			vTranslation = vTranslation * (fDistance / fDistanceZ0);

			layer->vTranslation.X = prev_x + vTranslation.X;
			layer->vTranslation.Y = prev_y + vTranslation.Y;

			break;
		}

		case PsController::TOOL_MODIFY_SIZE:
		{
			// FIXME : LOG !!

			tx = (fx - sx) * cos(-shape->GetAngle()) - (fy - sy) * sin(-shape->GetAngle());
			ty = (fx - sx) * sin(-shape->GetAngle()) + (fy - sy) * cos(-shape->GetAngle());

			/*
				if (init_corner == 0)
				layer->SetSize(prev_w - tx, prev_h - ty, -SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 1)
				layer->SetSize(prev_w + tx, prev_h - ty, SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 2)
				layer->SetSize(prev_w - tx, prev_h + ty, -SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else
				layer->SetSize(prev_w + tx, prev_h + ty, SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			*/
			break;
		}

		}
	}
	else
	{

		if (!shape || shape->hide)
			return;

		switch (tool)
		{
		case PsController::TOOL_MODIFY_MOVE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogMove(*this, shape, prev_x, prev_y));

			shape->SetPosition(prev_x + fx - sx, prev_y + fy - sy, constrain);
			break;
		}

		case PsController::TOOL_MODIFY_ROTATE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogRotate(*this, shape, prev_r, reflect));

			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((shape->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			shape->SetAngle(prev_r + shape->ToAngle(fx, fy) - shape->ToAngle(sx, sy), constrain, reflect);
			break;
		}

		case PsController::TOOL_MODIFY_SIZE:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogResize(*this, shape, prev_x, prev_y, prev_w, prev_h, reflect));

			tx = (fx - sx) * cos(-shape->GetAngle()) - (fy - sy) * sin(-shape->GetAngle());
			ty = (fx - sx) * sin(-shape->GetAngle()) + (fy - sy) * cos(-shape->GetAngle());

			if (init_corner == 0)
				shape->SetSize(prev_w - tx, prev_h - ty, -SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 1)
				shape->SetSize(prev_w + tx, prev_h - ty, SHAPE_SIZE, -SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else if (init_corner == 2)
				shape->SetSize(prev_w - tx, prev_h + ty, -SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			else
				shape->SetSize(prev_w + tx, prev_h + ty, SHAPE_SIZE, SHAPE_SIZE, prev_w, prev_h, constrain, reflect);
			break;
		}

		case PsController::TOOL_MODIFY_TORSIO:
		{
			if (this->LogMustAdd())
				this->LogAdd(new LogTorsio(*this, shape, prev_i, prev_j));

			tx = fx - sx;
			ty = fy - sy;

			if (init_corner == 0)
				shape->SetTorsion(prev_i - tx * cos(shape->GetAngle()) - ty * sin(shape->GetAngle()), prev_j, constrain);
			else if (init_corner == 1)
				shape->SetTorsion(prev_i, prev_j - tx * sin(shape->GetAngle()) + ty * cos(shape->GetAngle()), constrain);
			else if (init_corner == 2)
				shape->SetTorsion(prev_i + tx * cos(shape->GetAngle()) + ty * sin(shape->GetAngle()), prev_j, constrain);
			else
				shape->SetTorsion(prev_i, prev_j + tx * sin(shape->GetAngle()) - ty * cos(shape->GetAngle()), constrain);
			break;
		}
		}
	}

	PsController::Instance().UpdateWindow();
	if (dlgPropreties)
		dlgPropreties->UpdateInformation(this);
}

/*
** Scan de la zone de travail pour savoir quel curseur à afficher lors de l'utilisaton de l'outil
** "modify". Si le booleen "set" est à vrai, l'utilisateur a cliqué, donc on change de mode en plus
** de changer éventuellement de curseur.
*/
PsController::Tool PsProject::ToolModifyScan(int x, int y, bool set)
{
	float									fx;
	float									fy;

	renderer.Convert(x, y, fx, fy);

	if (bPatternsIsSelected && !pattern->hide)
	{
		PsLayer* layer = pattern->aLayers[iLayerId];

		/*
		if (layer->InResize(fx, fy, renderer.zoom, init_corner))
		 {
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 +(int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				prev_x = layer->vTranslation.X;
				prev_y = layer->vTranslation.Y;
				prev_h = layer->fScale * layer->fGizmoSize;
				prev_w = layer->fScale * layer->fGizmoSize;
			}

			return PsController::TOOL_MODIFY_SIZE;
		 }
		 */

		if (layer->InRotate(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				prev_r = layer->rRotation.Yaw;
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}

		if (layer->MouseIsInside(fx, fy))
		{
			if (set)
			{
				prev_x = layer->vTranslation.X;
				prev_y = layer->vTranslation.Y;

				/*
				** Préparation des paramètres de projection
				 */
				PsVector oeil = renderer.GetEyeLocation();
				PsVector direction(fx, fy, 0);
				direction -= oeil;
				PsVector normal(0.f, 0.f, 1.f);

				/*
				** Point d'intersection click souris & plan en z = 0
				 */
				PsVector vZ0 = layer->vTranslation;
				vZ0.Z = 0.f;
				PsVector* p = LinePlaneIntersection(oeil, direction, vZ0, normal);
				if (p)
				{
					prev_origin_z0 = *p;
					delete p;
				}

				/*
				** Point d'intersection click souris & PsLayer
				*/
				normal = RotateVertex(normal, PsRotator::FromDegree(layer->rRotation.Roll),
					PsRotator::FromDegree(layer->rRotation.Pitch), PsRotator::FromDegree(layer->rRotation.Yaw));
				p = LinePlaneIntersection(oeil, direction, layer->vTranslation, normal);
				if (p)
				{
					prev_origin = *p;
					delete p;
				}


			}

			PsController::Instance().SetCursor(CURSOR_MOVE);
			return PsController::TOOL_MODIFY_MOVE;
		}

		PsController::Instance().SetCursor(CURSOR_DEFAULT);
		return PsController::TOOL_MODIFY;
	}

	if (image && !image->hide && (!matrix || !matrix->hide))
	{
		if (image->InResize(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 + (int)((image->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				SelectImage(image);
				image->GetPosition(prev_x, prev_y);
				prev_h = image->h;
				prev_w = image->w;
			}

			return PsController::TOOL_MODIFY_SIZE;
		}
		else if (image->InRotate(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((image->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				SelectImage(image);
				prev_r = image->GetAngle();
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}
	}

	if (matrix && !matrix->hide)
	{
		if (matrix->InResize(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_SIZE1 + (int)((matrix->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 2));

			if (set)
			{
				SelectMatrix(matrix);
				matrix->GetPosition(prev_x, prev_y);
				prev_h = matrix->h;
				prev_w = matrix->w;
			}

			return PsController::TOOL_MODIFY_SIZE;
		}
		else if (matrix->InTorsion(fx, fy, renderer.zoom, init_corner))
		{
			if (set) PsController::Instance().SetCursor(CURSOR_TORSIO2);
			else PsController::Instance().SetCursor(CURSOR_TORSIO1);

			if (set)
			{
				SelectMatrix(matrix);
				prev_i = matrix->i;
				prev_j = matrix->j;
			}

			return PsController::TOOL_MODIFY_TORSIO;
		}
		else if (matrix->InRotate(fx, fy, renderer.zoom, init_corner))
		{
			PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((matrix->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

			if (set)
			{
				SelectMatrix(matrix);
				prev_r = matrix->r;
			}

			return PsController::TOOL_MODIFY_ROTATE;
		}
	}

	if (PsController::Instance().GetOption(PsController::OPTION_AUTOMATIC) || (!image && !matrix))
	{
		ImageList::reverse_iterator j;
		for (j = images.rbegin(); j != images.rend(); ++j)
			if (!(*j)->hide && (*j)->InContent(fx, fy))
			{
				PsController::Instance().SetCursor(CURSOR_MOVE);
				if (set)
				{
					SelectImage(*j);
					(*j)->GetPosition(prev_x, prev_y);
				}
				return PsController::TOOL_MODIFY_MOVE;
			}
		MatrixList::reverse_iterator i;
		for (i = matrices.rbegin(); i != matrices.rend(); ++i)
		{
			for (j = (*i)->images.rbegin(); !(*i)->hide && j != (*i)->images.rend(); ++j)
				if (!(*j)->hide && (*j)->InContent(fx, fy))
				{
					PsController::Instance().SetCursor(CURSOR_MOVE);
					if (set)
					{
						SelectImage(*j);
						(*j)->GetPosition(prev_x, prev_y);
					}
					return PsController::TOOL_MODIFY_MOVE;
				}
		}
		for (i = matrices.rbegin(); i != matrices.rend(); ++i)
			if (!(*i)->hide && (*i)->InContent(fx, fy))
			{
				PsController::Instance().SetCursor(CURSOR_MOVE);
				if (set)
				{
					SelectMatrix(*i);
					(*i)->GetPosition(prev_x, prev_y);
				}
				return PsController::TOOL_MODIFY_MOVE;
			}
	}
	else if (image && !image->hide && !matrix->hide)
	{
		PsController::Instance().SetCursor(CURSOR_MOVE);

		if (set)
		{
			SelectImage(image);
			image->GetPosition(prev_x, prev_y);
		}

		return PsController::TOOL_MODIFY_MOVE;
	}
	else if (matrix && !matrix->hide)
	{
		PsController::Instance().SetCursor(CURSOR_MOVE);

		if (set)
		{
			SelectMatrix(matrix);
			matrix->GetPosition(prev_x, prev_y);
		}

		return PsController::TOOL_MODIFY_MOVE;
	}

	PsController::Instance().SetCursor(CURSOR_DEFAULT);

	return PsController::TOOL_MODIFY;
}

/*
** Déplacement de l'outil "drag".
*/
void PsProject::ToolScrollDrag(int x, int y, int prev_x, int prev_y)
{
	renderer.SetScroll(prev_scrollx + (prev_x - x) * renderer.zoom, prev_scrolly + (prev_y - y) * renderer.zoom);
	PsController::Instance().UpdateWindow();
	PsController::Instance().UpdateDialogOverview();
}

/*
** Initialisation de l'outil "drag".
*/
PsController::Tool PsProject::ToolScrollStart()
{
	renderer.GetScroll(prev_scrollx, prev_scrolly);
	return PsController::TOOL_SCROLL_DRAG;
}

/*
** Retourne la largeur du document en pixels.
*/
int PsProject::GetWidth()
{
	return renderer.doc_x;
}

/*
** Retourne la hauteur du document en pixels.
*/
int PsProject::GetHeight()
{
	return renderer.doc_y;
}

/*
** Retourne le nombre de pixels par pouce dans le document.
*/
int PsProject::GetDpi()
{
	return renderer.dpi;
}