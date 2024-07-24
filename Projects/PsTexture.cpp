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

int PsTexture::max_resol = 512;
uint32 PsTexture::autogen_id_top = 1;

/*
** Constructor: by default, no texture is loaded.
*/
PsTexture::PsTexture() :
    buffer(0),
    id(0)
{
    this->autogen_id = autogen_id_top++;
}

/*
** Releases the texture if one was loaded, upon destruction.
*/
PsTexture::~PsTexture()
{
    this->LogFlush();
}

/*
** Unloads the loaded texture, if there was one.
*/
void PsTexture::LogFlush()
{
    if (this->buffer)
    {
        delete[] this->buffer;
        this->buffer = 0;
    }

    if (this->id)
    {
        glDeleteTextures(1, &this->id);
        this->id = 0;
    }

    PsWinProject::Instance().relaseThumb(this);
}

/*
** Returns the buffer in which the compressed texture was saved, and sets "size" to the size of
** the buffer in question. This is used when saving a texture to a file.
*/
uint8* PsTexture::GetBuffer(size_t& size) const
{
    size = 4 * sizeof(int) + *(int*)(this->buffer + 3 * sizeof(int));

    return this->buffer;
}

uint8* PsTexture::GetBufferUncompressed(int& bpp) const
{
    uint8* uncomp;
    int width;
    int height;
    uLongf size;

    width = *(int*)(this->buffer + 0 * sizeof(int));
    height = *(int*)(this->buffer + 1 * sizeof(int));
    bpp = *(int*)(this->buffer + 2 * sizeof(int));
    size = (width * height * bpp);

    uncomp = new uint8[size];
    int r = uncompress(uncomp, &size, this->buffer + 4 * sizeof(int), *(int*)(this->buffer + 3 * sizeof(int)));
    assert(r == Z_OK);
    return uncomp;
}

/*
** Returns the ID of the texture, loaded by OpenGL.
*/
GLuint PsTexture::GetID() const
{
    return this->id;
}

/*
** Retrieves the size of the currently loaded texture.
*/
void PsTexture::GetSize(int& x, int& y) const
{
    x = this->width;
    y = this->height;
}

/*
** Loads a texture from a compressed buffer (presumably from a file). See the function
** "LoadFromFile" to know how this buffer is organized.
*/
bool PsTexture::LoadFromBuffer(uint8* buffer)
{
    uint8* uncomp;
    int width;
    int height;
    int bpp;
    uLongf size;

    this->LogFlush();

    this->buffer = buffer;

    width = *(int*)(this->buffer + 0 * sizeof(int));
    height = *(int*)(this->buffer + 1 * sizeof(int));
    bpp = *(int*)(this->buffer + 2 * sizeof(int));
    size = width * height * bpp;

    uncomp = new uint8[size];

    if (uncompress(uncomp, &size, this->buffer + 4 * sizeof(int), *(int*)(this->buffer + 3 * sizeof(int))) != Z_OK)
        return false;

    if (!this->Register(width, height, bpp, uncomp))
        return false;

    delete[] uncomp;

    return true;
}

/*
** Loads a texture from a file in two different ways: an intact copy of the texture is
** kept in a buffer, and a resized version is saved for OpenGL. When saving,
** only the intact copy will be saved, the other can of course be constructed from it.
** The copy is compressed with zlib; it might be smart to use a real format suitable for images
** (and of course non-destructive, otherwise quality loss with each document save), such as
** PNG. The buffer contains, in order: the width, height, and bpp (divided by 8, see function
** "Register") on 4 bytes (int), then the size of the compressed data, also on 4 bytes, and finally
** the data itself. This gives us the formula found in the GetBuffer function to know
** the total size of the buffer: 4 * sizeof(int) + *(int*)(buffer + 3 * sizeof(int))
** FIXME: There will be a compatibility issue between Mac/PC, since one will save and read in big endian,
** while the other will be in little endian. We should replace all this with functions that convert an int
** to an array of 4 chars. On reflection, this problem will appear everywhere at the FileSave/FileLoad level...
*/
bool PsTexture::LoadFromFile(const char* path)
{
    int width, height, bpp;
    uint8* pixels;

    if (!FreeImage_BufferFromFile(path, pixels, width, height, bpp))
        return false;

    this->LogFlush();

    return this->RegisterAndSave(width, height, bpp, pixels);
}

/*
** Saves a copy of the image in memory in the internal format and then in OpenGL
*/
bool PsTexture::RegisterAndSave(int width, int height, int bpp, uint8* pixels)
{
    uLongf size = height * width * bpp * sizeof(uint8);
    this->buffer = new uint8[4 * sizeof(int) + size];

    if (compress(this->buffer + 4 * sizeof(int), &size, pixels, size) != Z_OK)
        return false;

    *(int*)(this->buffer + 0 * sizeof(int)) = width;
    *(int*)(this->buffer + 1 * sizeof(int)) = height;
    *(int*)(this->buffer + 2 * sizeof(int)) = bpp;
    *(int*)(this->buffer + 3 * sizeof(int)) = size;

    return this->Register(width, height, bpp, pixels);
}

void GetPixel24Bits(uint8* buffer, int w, int, int x, int y, int* color)
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

void GetPixel32Bits(uint8* buffer, int w, int, int x, int y, int* color)
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
** Transforms the texture for OpenGL, resizing it to the nearest power of 2, but with a
** maximum size. The original image size is width*height, it is in "bpp * 8" mode (i.e.: indicate
** 3 for 24 bpp, 4 for 32 bpp), and "pixels" is the array of pixels that compose it.
*/
bool PsTexture::Register(int width, int height, int bpp, uint8* pixels)
{
    TGetPixel getPixel;
    uint8* buffer;
    int color[4];
    int h;
    int i;
    int j;
    int n;
    int w;
    int x;
    int y;

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
                    // char a[928];
                    // sprintf(a, "read [%i, %i] on [%i, %i](%i, %i => %i, %i)\n", i, j, width, height, x * width / w, y * height / h,(x + 1) * width / w,(y + 1) * height / h);
                    // OutputDebugString(a);

                    (*getPixel)(pixels, width, height, i, j, color);
                    ++n;
                } while (++j < (y + 1) * height / h);
            } while (++i < (x + 1) * width / w);

            buffer[(x + y * w) * 4 + 0] = color[0] / n;
            buffer[(x + y * w) * 4 + 1] = color[1] / n;
            buffer[(x + y * w) * 4 + 2] = color[2] / n;
            buffer[(x + y * w) * 4 + 3] = color[3] / n;
        }

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);

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
** Unloads and then reloads the texture in OpenGL.
*/
void PsTexture::Reload()
{
    uint8* backup;
    uint8* buffer;
    size_t size;

    buffer = this->GetBuffer(size);

    backup = new uint8[size];
    memcpy(backup, buffer, size * sizeof(*backup));

    this->LoadFromBuffer(backup);

    delete[] backup;
}

/*
** Changes the maximum resolution of textures for OpenGL
*/
void PsTexture::SetMaxResol(int resol)
{
    max_resol = resol;
}
