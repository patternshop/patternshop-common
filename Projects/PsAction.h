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

#include "PsImage.h"
#include "PsMatrix.h"
#include "PsProjectController.h"

class	PsAction
{
	public:
		/**/					PsAction (PsProjectController&, PsShape*);
		virtual					~PsAction();

		virtual PsAction*		Execute() = 0;
		virtual const char*		Name() const = 0;

	protected:
		PsImage*				GetImage (ImageList::iterator&) const;
		PsMatrix*				GetMatrix (MatrixList::iterator&) const;
		PsShape*				GetShape() const;

		PsProjectController&				project;
		int						matrix;
		int						image;
};

class	LogDelImage : public PsAction
{
	public:
		/**/				LogDelImage (PsProjectController&, PsImage*, bool);
		/**/				~LogDelImage();

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		bool				create;
		uint8*				buffer;
		size_t				size;
		float				h;
		float				i;
		float				j;
		float				r;
		float				w;
		float				x;
		float				y;
};

class	LogDelMatrix : public PsAction
{
	public:
		/**/				LogDelMatrix (PsProjectController&, PsMatrix*, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		bool				create;
		bool				div_is_active;
		int					div_x;
		int					div_y;
		int					color;
		float				h;
		float				i;
		float				j;
		float				r;
		float				w;
		float				x;
		float				y;
};

class	LogMove : public PsAction
{
	public:
		/**/				LogMove (PsProjectController&, PsShape*, float, float);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				x;
		float				y;
};

class	LogNewImage : public PsAction
{
	public:
		/**/				LogNewImage (PsProjectController&, PsImage*, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		bool				create;
};

class	LogNewMatrix : public PsAction
{
	public:
		/**/				LogNewMatrix (PsProjectController&, PsMatrix*, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		bool				create;
};

class	LogReplace : public PsAction
{
	public:
		/**/				LogReplace (PsProjectController&, PsImage*);
		/**/				~LogReplace();

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		uint8*		buffer;
		size_t				size;
};

class	LogResize : public PsAction
{
	public:
		/**/				LogResize (PsProjectController&, PsShape*, float, float, float, float);
		/**/				LogResize (PsProjectController&, PsShape*, float, float, float, float, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				h;
		float				w;
		float				x;
		float				y;
		bool				reflect;
};

class	LogRotate : public PsAction
{
	public:
		/**/				LogRotate (PsProjectController&, PsShape*, float);
		/**/				LogRotate (PsProjectController&, PsShape*, float, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				r;
		bool				reflect;
};

class	LogSwapImage : public PsAction
{
	public:
		/**/				LogSwapImage (PsProjectController&, PsShape*, PsMatrix*, PsMatrix*, int);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		int					from;
		int					swap;
		int					to;
};

class	LogSwapMatrix : public PsAction
{
	public:
		/**/				LogSwapMatrix (PsProjectController&, PsShape*, int);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		int					swap;
};

class	LogTorsio : public PsAction
{
	public:
		/**/				LogTorsio (PsProjectController&, PsShape*, float, float);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				i;
		float				j;
};

class	LogPatternAction : public PsAction
{
	public:
		/**/				LogPatternAction (PsProjectController&);

		virtual PsAction*	Execute() = 0;
		virtual const char*	Name() const = 0;

	protected:
		PsLayer				*GetCurrentLayer();

	protected:
		int					iSelectedLayer;
};

class	LogPatternRotate : public LogPatternAction
{
	public:
		/**/				LogPatternRotate (PsProjectController&);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		PsRotator			rRotation;
};

class	LogPatternTranslate : public LogPatternAction
{
	public:
		/**/				LogPatternTranslate (PsProjectController&);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		PsVector			vTranslation;
};

class	LogPatternScale : public LogPatternAction
{
	public:
		/**/				LogPatternScale (PsProjectController&);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				fScale;
};
