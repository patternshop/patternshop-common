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
#include <assert.h>
#include "zlib.h"

#include "PsTexture.h"
#include "PsSoftRender.h"
#include "PsWinProject.h"

bool FreeImage_BufferFromFile(const char*, uint8*&, int&, int&, int&);

int	PsTexture::max_resol = 512;
uint32 PsTexture::autogen_id_top = 1;

/*
** Constructeur : par défaut, aucune texture chargée.
*/
PsTexture::PsTexture() :
	buffer(0),
	id(0)
{
	autogen_id = autogen_id_top++;
}

/*
** Liberation de la texture éventuellement chargée, lors de la destruction.
*/
PsTexture::~PsTexture()
{
	LogFlush();
}

/*
** Décharge la texture chargée, si il y en avait une.
*/
void	PsTexture::LogFlush()
{
	if (buffer)
	{
		delete[] buffer;
		buffer = 0;
	}

	if (id)
	{
		glDeleteTextures(1, &id);
		id = 0;
	}

	PsWinProject::Instance().relaseThumb(this);
}

/*
** Retourne le buffer dans lequel a été enregistrée la texture compressée, et fixe "size" à la taille du
** buffer en question. Ceci est utilisé lors de la sauvegarde d'une texture dans un fichier.
*/
uint8* PsTexture::GetBuffer(size_t& size) const
{
	size = 4 * sizeof(int) + *(int*)(buffer + 3 * sizeof(int));

	return buffer;
}

uint8* PsTexture::GetBufferUncompressed(int& bpp) const
{
	uint8* uncomp;
	int		width;
	int		height;
	uLongf	size;

	width = *(int*)(buffer + 0 * sizeof(int));
	height = *(int*)(buffer + 1 * sizeof(int));
	bpp = *(int*)(buffer + 2 * sizeof(int));
	size = (width * height * bpp);

	uncomp = new uint8[size];
	int r = uncompress(uncomp, &size, buffer + 4 * sizeof(int), *(int*)(buffer + 3 * sizeof(int)));
	assert(r == Z_OK);
	return uncomp;

}

/*
** Retourne l'ID de la texture, chargée par OpenGL.
*/
GLuint	PsTexture::GetID() const
{
	return this->id;
}

/*
** Récupere la taille de la texture actuellement chargée.
*/
void	PsTexture::GetSize(int& x, int& y) const
{
	x = this->width;
	y = this->height;
}

/*
** Charge une texture depuis un buffer compressé(donc a priori depuis un fichier). Voir la fonction
** "LoadFromFile" pour savoir comment est organisé ce buffer.
*/
bool	PsTexture::LoadFromBuffer(uint8* buffer)
{
	uint8* uncomp;
	int			width;
	int			height;
	int			bpp;
	uLongf		size;

	LogFlush();

	this->buffer = buffer;

	width = *(int*)(buffer + 0 * sizeof(int));
	height = *(int*)(buffer + 1 * sizeof(int));
	bpp = *(int*)(buffer + 2 * sizeof(int));
	size = width * height * bpp;

	uncomp = new uint8[size];

	if (uncompress(uncomp, &size, buffer + 4 * sizeof(int), *(int*)(buffer + 3 * sizeof(int))) != Z_OK)
		return false;

	if (!Register(width, height, bpp, uncomp))
		return false;

	delete[] uncomp;

	return true;
}

/*
** Charge une texture depuis un fichier, de deux façon différentes : une copie intacte de la texture est
** conservée dans un buffer, et une version redimentionnée est enregistrée pour OpenGL. Lors de la sauvegarde,
** seule la copie intacte sera sauvegardée, l'autre pouvant bien sûr se constuire à partir d'elle.
** La copie est compressée avec zlib; il pourrait être intelligent d'utiliser un vrai format adapté aux images
**(et bien sûr non destructif, sinon perte de qualité à chaque sauvegarde du document), comme par exemple le
** PNG. Le buffer contient, dans l'ordre : la largeur, la hauteur et les bpp(divisés par 8, cf fonction
** "Register") sur 4 octets(int), puis la taille des données compressées, sur 4 octets également, et enfin
** les données elles-même. Ceci nous donne la formule que l'on retrouve dans la fonction GetBuffer pour connaitre
** la taille totale du buffer : 4 * sizeof(int) + *(int*)(buffer + 3 * sizeof(int))
** FIXME : Il va y avoir un problème de compatibilité Mac/PC, puisque l'un va sauver et lire en big endian, alors
** que l'autre sera en little endian. Il faudrait remplacer tout ça par des fonctions qui convertissent un int
** en un tableau de 4 chars. À la reflexion, se problème va apparaitre partout au niveau des FileSave/FileLoad...
*/
bool		PsTexture::LoadFromFile(const char* path)
{
	int width, height, bpp;
	uint8* pixels;

	if (!FreeImage_BufferFromFile(path, pixels, width, height, bpp))
		return false;

	LogFlush();

	return RegisterAndSave(width, height, bpp, pixels);
}

/*
** Enregistre une copie de l'image en mémoire dans le format interne puis en openGL
*/
bool	PsTexture::RegisterAndSave(int width, int height, int bpp, uint8* pixels)
{
	uLongf	size = height * width * bpp * sizeof(uint8);
	buffer = new uint8[4 * sizeof(int) + size];

	if (compress(this->buffer + 4 * sizeof(int), &size, pixels, size) != Z_OK)
		return false;

	*(int*)(this->buffer + 0 * sizeof(int)) = width;
	*(int*)(this->buffer + 1 * sizeof(int)) = height;
	*(int*)(this->buffer + 2 * sizeof(int)) = bpp;
	*(int*)(this->buffer + 3 * sizeof(int)) = size;

	return Register(width, height, bpp, pixels);
}

void	GetPixel24Bits(uint8* buffer, int w, int, int x, int y, int* color)
{
#ifndef __BIG_ENDIAN__
	color[0] += buffer[(x + y * w) * 3 + 2];
	color[2] += buffer[(x + y * w) * 3 + 0];
#else
	color[0] += buffer[(x + y * w) * 3 + 0];
	color[2] += buffer[(x + y * w) * 3 + 2];
#endif
	color[1] += buffer[(x + y * w) * 3 + 1];
	color[3] += 255;
}

void	GetPixel32Bits(uint8* buffer, int w, int, int x, int y, int* color)
{
#ifndef __BIG_ENDIAN__	
	color[0] += buffer[(x + y * w) * 4 + 2];
	color[2] += buffer[(x + y * w) * 4 + 0];
#else
	color[0] += buffer[(x + y * w) * 4 + 0];
	color[2] += buffer[(x + y * w) * 4 + 2];
#endif
	color[1] += buffer[(x + y * w) * 4 + 1];
	color[3] += buffer[(x + y * w) * 4 + 3];
}

/*
** Transforme la texture pour OpenGL, en la redimentionnant à la puissance de 2 la plus proche, mais avec une
** taille maximum. La taille de l'image originale est width*height, elle est en mode "bpp * 8" (i.e : indiquer
** 3 pour 24 bpp, 4 pour 32 bpp), et "pixels" est le tableau des pixels qui la composent.
*/
bool	PsTexture::Register(int width, int height, int bpp, uint8* pixels)
{
	TGetPixel	getPixel;
	uint8*		buffer;
	int			color[4];
	int			h;
	int			i;
	int			j;
	int			n;
	int			w;
	int			x;
	int			y;

	switch (bpp)
	{
	case 3: getPixel = GetPixel24Bits; break;
	case 4: getPixel = GetPixel32Bits; break;
	default: return false;
	}

	for (h = 1; h < height && h < max_resol; )
		h <<= 1;

	for (w = 1; w < width && w < max_resol; )
		w <<= 1;

	buffer = new uint8[w * h * 4];
	this->width = width;
	this->height = height;

	for (x = w; x--; )
		for (y = h; y--; )
		{
			color[0] = 0;
			color[1] = 0;
			color[2] = 0;
			color[3] = 0;

			i = x * width / w;
			n = 0;

			do
			{
				j = y * height / h;

				do
				{
					//					char a[928];
					//					sprintf(a, "read [%i, %i] on [%i, %i](%i, %i => %i, %i)\n", i, j, width, height, x * width / w, y * height / h,(x + 1) * width / w,(y + 1) * height / h);
					//					OutputDebugString(a);

					(*getPixel)(pixels, width, height, i, j, color);
					++n;
				} while (++j < (y + 1) * height / h);
			} while (++i < (x + 1) * width / w);

			buffer[(x + y * w) * 4 + 0] = color[0] / n;
			buffer[(x + y * w) * 4 + 1] = color[1] / n;
			buffer[(x + y * w) * 4 + 2] = color[2] / n;
			buffer[(x + y * w) * 4 + 3] = color[3] / n;
		}

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

#ifdef _WINDOWS
	bool linear = true;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
#else /* MAXOSX */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#endif

	delete[] buffer;

	return true;
}

/*
** Décharge puis recharge la texture dans OpenGL.
*/
void				PsTexture::Reload()
{
	uint8* backup;
	uint8* buffer;
	size_t		size;

	buffer = GetBuffer(size);

	backup = new uint8[size];
	memcpy(backup, buffer, size * sizeof(*backup));

	LoadFromBuffer(backup);

	delete[] backup;
}

/*
** Change la résolution maximale des textures pour OpenGL
*/
void		PsTexture::SetMaxResol(int resol)
{
	max_resol = resol;
}
