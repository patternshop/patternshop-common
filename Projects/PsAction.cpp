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
#include "PsController.h"
#include "PsAction.h"
#include <assert.h>

PsAction::PsAction(PsProject& project, PsShape* shape) :
	project(project)
{
	MatrixList::iterator	tm;
	ImageList::iterator		ti;

	this->matrix = 0;

	for (tm = this->project.matrices.begin(); tm != this->project.matrices.end() && *tm != shape; ++tm)
	{
		this->image = 0;

		for (ti = (*tm)->images.begin(); ti != (*tm)->images.end() && *ti != shape; ++ti)
			++this->image;

		if (ti != (*tm)->images.end())
			return;

		++this->matrix;
	}

	this->image = -1;

	if (tm != this->project.matrices.end())
		return;

	this->image = 0;
	this->matrix = -1;

	for (ti = this->project.images.begin(); ti != this->project.images.end() && *ti != shape; ++ti)
		++this->image;

	if (ti != this->project.images.end())
		return;

	this->image = -1;
}

PsAction::~PsAction()
{
}

PsImage* PsAction::GetImage(ImageList::iterator& ti) const
{
	MatrixList::iterator	tm;
	int		vi;
	int		vm;

	vi = this->image;
	vm = this->matrix;

	if (vi != -1)
	{
		if (vm != -1)
		{
			for (tm = this->project.matrices.begin(); tm != this->project.matrices.end() && vm--; )
				++tm;

			if (tm != this->project.matrices.end())
			{
				for (ti = (*tm)->images.begin(); ti != (*tm)->images.end() && vi--; )
					++ti;

				if (ti != (*tm)->images.end())
					return *ti;
			}
		}
		else
		{
			for (ti = this->project.images.begin(); ti != this->project.images.end() && vi--; )
				++ti;

			if (ti != this->project.images.end())
				return *ti;
		}
	}

	return 0;
}

PsMatrix* PsAction::GetMatrix(MatrixList::iterator& tm) const
{
	int		vm;

	vm = this->matrix;

	if (vm != -1)
	{
		for (tm = this->project.matrices.begin(); tm != this->project.matrices.end() && vm--; )
			++tm;

		if (tm != this->project.matrices.end())
			return *tm;
	}

	return 0;
}

PsShape* PsAction::GetShape() const
{
	ImageList::iterator		ti;
	MatrixList::iterator	tm;
	PsShape* shape;

	if ((shape = this->GetImage(ti)))
		return shape;
	else if ((shape = this->GetMatrix(tm)))
		return shape;

	return 0;
}

LogDelImage::LogDelImage(PsProject& project, PsImage* image, bool create) :
	PsAction(project, image),
	create(create)
{
	uint8* buffer;

	buffer = image->GetTexture().GetBuffer(this->size);

	this->buffer = new uint8[this->size];
	memcpy(this->buffer, buffer, this->size * sizeof(*this->buffer));
	image->GetPosition(this->x, this->y);
	this->r = image->GetAngle();
	this->h = image->h;
	this->i = image->i;
	this->j = image->j;
	this->w = image->w;
}

LogDelImage::~LogDelImage()
{
	delete[] this->buffer;
}

PsAction* LogDelImage::Execute()
{
	ImageList::iterator		ti;
	MatrixList::iterator	tm;
	PsImage* image;
	PsMatrix* matrix;
	uint8* buffer;

	if (this->image != -1)
	{
		matrix = this->GetMatrix(tm);
		image = new PsImage(matrix);

		this->GetImage(ti);

		if (matrix)
			matrix->images.insert(ti, image);
		else
			this->project.images.insert(ti, image);

		buffer = new uint8[this->size];
		memcpy(buffer, this->buffer, this->size * sizeof(*buffer));
		image->TextureFromBuffer(buffer);
		image->SetSize(this->w, this->h);
		image->SetPosition(this->x, this->y);
		image->SetAngle(this->r);
		image->SetTorsion(this->i, this->j);

		this->project.SelectImage(image);

		return new LogNewImage(this->project, image, this->create);
	}

	return 0;
}

const char* LogDelImage::Name() const
{
	if (this->create != false)
	{
		return GetAction(ACTION_NEW_IMAGE);
	}
	else
	{
		return GetAction(ACTION_DEL_IMAGE);
	}
}

LogDelMatrix::LogDelMatrix(PsProject& project,
	PsMatrix* matrix,
	bool create)
	: PsAction(project,
		matrix),
	create(create)
{
	matrix->GetPosition(this->x, this->y);
	this->color = matrix->color;
	this->div_is_active = matrix->div_is_active;
	this->div_x = matrix->div_x;
	this->div_y = matrix->div_y;
	this->r = matrix->GetAngle();
	this->h = matrix->h;
	this->i = matrix->i;
	this->j = matrix->j;
	this->w = matrix->w;
}

PsAction* LogDelMatrix::Execute()
{
	MatrixList::iterator	tm;
	PsMatrix* matrix;

	if (this->matrix != -1)
	{
		matrix = new PsMatrix();

		this->GetMatrix(tm);

		this->project.matrices.insert(tm, matrix);

		matrix->SetSize(this->w, this->h);
		matrix->SetPosition(this->x, this->y);
		matrix->SetAngle(this->r);
		matrix->SetTorsion(this->i, this->j);
		matrix->color = this->color;
		matrix->div_is_active = this->div_is_active;
		matrix->div_x = this->div_x;
		matrix->div_y = this->div_y;

		this->project.SelectMatrix(matrix);

		return new LogNewMatrix(this->project, matrix, this->create);
	}

	return 0;
}

const char* LogDelMatrix::Name() const
{
	if (this->create != false)
	{
		return GetAction(ACTION_NEW_MATRIX);
	}
	else
	{
		return GetAction(ACTION_DEL_MATRIX);
	}
}

LogMove::LogMove(PsProject& project, PsShape* shape, float x, float y) :
	PsAction(project, shape),
	x(x),
	y(y)
{
}

PsAction* LogMove::Execute()
{
	PsShape* shape;
	PsAction* log;
	float	x;
	float	y;

	if ((shape = this->GetShape()))
	{
		shape->GetPosition(x, y);

		log = new LogMove(this->project, shape, x, y);
		shape->SetPosition(this->x, this->y);

		return log;
	}

	return 0;
}

const char* LogMove::Name() const
{
	return GetAction(ACTION_MOVE);
}

LogNewImage::LogNewImage(PsProject& project, PsImage* image, bool create) :
	PsAction(project, image),
	create(create)
{
}

PsAction* LogNewImage::Execute()
{
	ImageList::iterator		ti;
	MatrixList::iterator	tm;
	PsImage* image;
	PsMatrix* matrix;
	PsAction* log;

	log = NULL;

	if (this->matrix == -1)
	{
		if ((image = this->GetImage(ti)))
		{
			log = new LogDelImage(this->project, image, this->create);
			this->project.images.erase(ti);

			delete image;
		}
	}
	else
	{
		if ((matrix = this->GetMatrix(tm)) && (image = this->GetImage(ti)))
		{
			log = new LogDelImage(this->project, image, this->create);
			matrix->images.erase(ti);

			delete image;
		}
	}

	this->project.SelectImage(0);

	return log;
}

const char* LogNewImage::Name() const
{
	if (this->create != false)
	{
		return GetAction(ACTION_NEW_IMAGE);
	}
	else
	{
		return GetAction(ACTION_DEL_IMAGE);
	}
}

LogNewMatrix::LogNewMatrix(PsProject& project, PsMatrix* matrix, bool create) :
	PsAction(project, matrix),
	create(create)
{
}

PsAction* LogNewMatrix::Execute()
{
	MatrixList::iterator	tm;
	PsMatrix* matrix;
	PsAction* log;

	log = NULL;

	if ((matrix = this->GetMatrix(tm)))
	{
		log = new LogDelMatrix(this->project, matrix, this->create);
		this->project.matrices.erase(tm);

		delete matrix;
	}

	this->project.SelectMatrix(0);

	return log;
}

const char* LogNewMatrix::Name() const
{
	if (this->create != false)
	{
		return GetAction(ACTION_NEW_MATRIX);
	}
	else
	{
		return GetAction(ACTION_DEL_MATRIX);
	}
}

LogReplace::LogReplace(PsProject& project, PsImage* image) :
	PsAction(project, image)
{
	uint8* buffer;

	buffer = image->GetTexture().GetBuffer(this->size);

	this->buffer = new uint8[this->size];
	memcpy(this->buffer, buffer, this->size * sizeof(*this->buffer));
}

LogReplace::~LogReplace()
{
	delete[] this->buffer;
}

PsAction* LogReplace::Execute()
{
	ImageList::iterator	ti;
	PsImage* image;
	PsAction* log;
	uint8* buffer;

	if ((image = this->GetImage(ti)))
	{
		log = new LogReplace(this->project, image);

		buffer = new uint8[this->size];
		memcpy(buffer, this->buffer, this->size * sizeof(*buffer));
		image->TextureFromBuffer(buffer, false);

		return log;
	}

	return 0;
}

const char* LogReplace::Name() const
{
	return GetAction(ACTION_REPLACE);
}

LogResize::LogResize(PsProject& project,
	PsShape* shape,
	float x,
	float y,
	float w,
	float h)
	: PsAction(project,
		shape),
	h(h),
	w(w),
	x(x),
	y(y)
{
	reflect = PsController::Instance().GetOption(PsController::OPTION_REFLECT);
}

LogResize::LogResize(PsProject& project, PsShape* shape, float x, float y, float w, float h, bool reflect) :
	PsAction(project, shape),
	h(h),
	w(w),
	x(x),
	y(y),
	reflect(reflect)
{
}

PsAction* LogResize::Execute()
{
	PsShape* shape;
	PsAction* log;
	float	x;
	float	y;

	if ((shape = this->GetShape()))
	{
		shape->GetPosition(x, y);

		log = new LogResize(this->project, shape, x, y, shape->w, shape->h, this->reflect);
		shape->SetSize(this->w, this->h, 0, 0, 0, 0, false, this->reflect);
		shape->SetPosition(this->x, this->y);

		return log;
	}

	return 0;
}

const char* LogResize::Name() const
{
	return GetAction(ACTION_RESIZE);
}

LogRotate::LogRotate(PsProject& project, PsShape* shape, float r) :
	PsAction(project, shape),
	r(r)
{
	reflect = PsController::Instance().GetOption(PsController::OPTION_REFLECT);
}

LogRotate::LogRotate(PsProject& project, PsShape* shape, float r, bool reflect) :
	PsAction(project, shape),
	r(r),
	reflect(reflect)
{
}

PsAction* LogRotate::Execute()
{
	PsShape* shape;
	PsAction* log;
	float	r;

	if ((shape = this->GetShape()))
	{
		r = shape->GetAngle();

		log = new LogRotate(this->project, shape, r, this->reflect);
		shape->SetAngle(this->r, false, this->reflect);

		return log;
	}

	return 0;
}

const char* LogRotate::Name() const
{
	return GetAction(ACTION_ROTATE);
}

LogSwapImage::LogSwapImage(PsProject& project, PsShape* shape, PsMatrix* from, PsMatrix* to, int swap) :
	PsAction(project, shape),
	swap(swap)
{
	MatrixList::iterator	tm;
	ImageList::iterator		ti;
	int						i;

	this->from = -1;
	this->to = -1;

	for (i = 0, tm = this->project.matrices.begin(); tm != this->project.matrices.end(); ++tm)
	{
		if (*tm == from)
			this->from = i;
		if (*tm == to)
			this->to = i;

		++i;
	}
}

PsAction* LogSwapImage::Execute()
{
	MatrixList::iterator	tm;
	PsMatrix* from;
	PsMatrix* to;
	ImageList::iterator		ti;
	ImageList* list;
	PsImage* image;
	int						i;

	if ((image = this->GetImage(ti)))
	{
		for (i = this->from, tm = this->project.matrices.begin(); tm != this->project.matrices.end() && i--; )
			++tm;

		from = (tm == this->project.matrices.end() ? 0 : *tm);

		for (i = this->to, tm = this->project.matrices.begin(); tm != this->project.matrices.end() && i--; )
			++tm;

		to = (tm == this->project.matrices.end() ? 0 : *tm);

		list = from ? &from->images : &this->project.images;
		list->remove(image);

		list = to ? &to->images : &this->project.images;

		for (i = this->swap, ti = list->begin(); ti != list->end() && i--; )
			++ti;

		list->insert(ti, image);
		image->parent = to;

		if (image->parent)
		{
			image->x = 0;
			image->y = 0;
		}
		else if (from || to)
			image->SetPosition(this->project.GetWidth() / 2.0f, this->project.GetHeight() / 2.0f);

		PsController::Instance().UpdateDialogProject();

		return new LogSwapImage(this->project, image, to, from, this->image);
	}

	return 0;
}

const char* LogSwapImage::Name() const
{
	return GetAction(ACTION_SWAP_IMAGE);
}

LogSwapMatrix::LogSwapMatrix(PsProject& project, PsShape* shape, int swap) :
	PsAction(project, shape),
	swap(swap)
{
}

PsAction* LogSwapMatrix::Execute()
{
	MatrixList::iterator	tm;
	PsMatrix* matrix;
	int						i;

	if ((matrix = this->GetMatrix(tm)))
	{
		this->project.matrices.remove(matrix);

		for (i = this->swap, tm = this->project.matrices.begin(); tm != this->project.matrices.end() && i--; )
			++tm;

		this->project.matrices.insert(tm, matrix);

		PsController::Instance().UpdateDialogProject();

		return new LogSwapMatrix(this->project, matrix, this->matrix);
	}

	return 0;
}

const char* LogSwapMatrix::Name() const
{
	return GetAction(ACTION_SWAP_IMAGE);
}

LogTorsio::LogTorsio(PsProject& project, PsShape* shape, float i, float j) :
	PsAction(project, shape),
	i(i),
	j(j)
{
}

PsAction* LogTorsio::Execute()
{
	PsShape* shape;
	PsAction* log;

	if ((shape = this->GetShape()))
	{
		log = new LogTorsio(this->project, shape, shape->i, shape->j);
		shape->SetTorsion(this->i, this->j);

		return log;
	}

	return 0;
}

const char* LogTorsio::Name() const
{
	return GetAction(ACTION_TORSIO);
}

LogPatternAction::LogPatternAction(PsProject& project) :
	PsAction(project, NULL)
{
	assert(project.pattern);
	iSelectedLayer = project.iLayerId;
}

PsLayer* LogPatternAction::GetCurrentLayer()
{
	assert(project.pattern);
	assert(iSelectedLayer >= 0);
	assert(iSelectedLayer < project.pattern->aLayers.size());
	return project.pattern->aLayers[iSelectedLayer];
}

LogPatternRotate::LogPatternRotate(PsProject& project) :
	LogPatternAction(project)
{
	rRotation = GetCurrentLayer()->rRotation;
}

PsAction* LogPatternRotate::Execute()
{
	PsAction* log = new LogPatternRotate(this->project);
	GetCurrentLayer()->rRotation = rRotation;
	return log;
}

const char* LogPatternRotate::Name() const
{
	return GetAction(ACTION_PATTERN_ROTATE);
}

LogPatternTranslate::LogPatternTranslate(PsProject& project) :
	LogPatternAction(project)
{
	vTranslation = GetCurrentLayer()->vTranslation;
}

PsAction* LogPatternTranslate::Execute()
{
	PsAction* log = new LogPatternTranslate(this->project);
	GetCurrentLayer()->vTranslation = vTranslation;
	return log;
}

const char* LogPatternTranslate::Name() const
{
	return GetAction(ACTION_PATTERN_TRANSLATE);
}

LogPatternScale::LogPatternScale(PsProject& project) :
	LogPatternAction(project)
{
	fScale = GetCurrentLayer()->fScale;
}

PsAction* LogPatternScale::Execute()
{
	PsAction* log = new LogPatternScale(this->project);
	GetCurrentLayer()->fScale = fScale;
	return log;
}

const char* LogPatternScale::Name() const
{
	return GetAction(ACTION_PATTERN_SCALE);
}
