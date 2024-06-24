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
#ifndef PS_ACTION_H___

#define PS_ACTION_H___

#include "PsImage.h"
#include "PsMatrix.h"
#include "PsProject.h"

class	PsAction
{
	public:
		/**/					PsAction (PsProject&, PsShape*);
		virtual					~PsAction();

		virtual PsAction*		Execute() = 0;
		virtual const char*		Name() const = 0;

	protected:
		PsImage*				GetImage (ImageList::iterator&) const;
		PsMatrix*				GetMatrix (MatrixList::iterator&) const;
		PsShape*				GetShape() const;

		PsProject&				project;
		int						matrix;
		int						image;
};

class	LogDelImage : public PsAction
{
	public:
		/**/				LogDelImage (PsProject&, PsImage*, bool);
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
		/**/				LogDelMatrix (PsProject&, PsMatrix*, bool);

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
		/**/				LogMove (PsProject&, PsShape*, float, float);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				x;
		float				y;
};

class	LogNewImage : public PsAction
{
	public:
		/**/				LogNewImage (PsProject&, PsImage*, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		bool				create;
};

class	LogNewMatrix : public PsAction
{
	public:
		/**/				LogNewMatrix (PsProject&, PsMatrix*, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		bool				create;
};

class	LogReplace : public PsAction
{
	public:
		/**/				LogReplace (PsProject&, PsImage*);
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
		/**/				LogResize (PsProject&, PsShape*, float, float, float, float);
		/**/				LogResize (PsProject&, PsShape*, float, float, float, float, bool);

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
		/**/				LogRotate (PsProject&, PsShape*, float);
		/**/				LogRotate (PsProject&, PsShape*, float, bool);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				r;
		bool				reflect;
};

class	LogSwapImage : public PsAction
{
	public:
		/**/				LogSwapImage (PsProject&, PsShape*, PsMatrix*, PsMatrix*, int);

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
		/**/				LogSwapMatrix (PsProject&, PsShape*, int);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		int					swap;
};

class	LogTorsio : public PsAction
{
	public:
		/**/				LogTorsio (PsProject&, PsShape*, float, float);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				i;
		float				j;
};

class	LogPatternAction : public PsAction
{
	public:
		/**/				LogPatternAction (PsProject&);

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
		/**/				LogPatternRotate (PsProject&);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		PsRotator			rRotation;
};

class	LogPatternTranslate : public LogPatternAction
{
	public:
		/**/				LogPatternTranslate (PsProject&);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		PsVector			vTranslation;
};

class	LogPatternScale : public LogPatternAction
{
	public:
		/**/				LogPatternScale (PsProject&);

		virtual PsAction*	Execute();
		virtual const char*	Name() const;

	private:
		float				fScale;
};

#endif /* PS_ACTION_H___ */

