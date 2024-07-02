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

#ifndef PS_TEXTURE_H__

#define PS_TEXTURE_H__

#include "PsOpenGL.h"

#include "PsTypes.h"

typedef	void	(*TGetPixel)(uint8*, int, int, int, int, int*);

/*
** La classe PsTexture sert bien sûr à charger des textures, mais a une double utilité. Elle
** garde une copie intacte de la texture pour l'utiliser en rendu software, et charge en même
** temps une copie optimisée pour OpenGL.
*/
class	PsTexture
{
public:
	PsTexture();
	~PsTexture();

	bool			LoadFromBuffer(uint8*);
	bool			LoadFromFile(const char*);

	uint8* GetBuffer(size_t&) const;
	GLuint			GetID() const;
	void			GetSize(int&, int&) const;

	void			Reload();

	static void		SetMaxResol(int);

	uint8* GetBufferUncompressed(int& bpp) const;

	int				GetBits() { return *(int*)(buffer + 2 * sizeof(int)); }

	const uint32	GetAutoGenId() { return autogen_id; }

public:
	void			LogFlush();
	bool			Register(int, int, int, uint8*);
	bool			RegisterAndSave(int, int, int, uint8*);

	static int		max_resol;
	uint8* buffer;
	GLuint			id;
	int				width;
	int				height;

private: // identifiant unique de texture
	uint32	autogen_id;
	static uint32	autogen_id_top;
};

void GetPixel24Bits(uint8* buffer, int w, int, int x, int y, int* color);
void GetPixel32Bits(uint8* buffer, int w, int, int x, int y, int* color);

#endif /* PS_TEXTURE_H__ */


