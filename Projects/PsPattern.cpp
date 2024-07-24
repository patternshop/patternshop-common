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
#include "PsPattern.h"
#include "tiffio.h"
#include "FreeImage.h"

float PsPattern::minimumAlpha = 240;

PsPattern::PsPattern()
{
	/*
	 color[0] = -1;
	 color[1] = -1;
	 color[2] = -1;
	 */
	this->hide = false;
	this->light_power = 0.0f;
	this->light_range = 1.0f;
	this->y_map_texture_id = 0;
	this->y_map_texture_data = 0;
}

PsPattern::~PsPattern()
{
	if (this->y_map_texture_id)
	{
		glDeleteTextures(1, &this->y_map_texture_id);
		this->y_map_texture_id = 0;
	}
	if (this->y_map_texture_data)
	{
		delete[] this->y_map_texture_data;
		this->y_map_texture_data = 0;
	}
	for (int i = 0; i < this->aLayers.size(); ++i)
	{
		delete this->aLayers[i];
		this->aLayers[i] = 0;
	}
	this->aLayers.clear();
}

/**
 * Load the pattern content from a file.
*/
ErrID PsPattern::FileLoad(FILE* file)
{
	uint8* buffer;
	size_t size;

	if (fread(&size, sizeof(size), 1, file) != 1)
		return ERROR_FILE_READ;

	buffer = new uint8[size];

	if (fread(buffer, sizeof(*buffer), size, file) != size)
		return ERROR_FILE_READ;

	if (fread(&this->hide, sizeof(this->hide), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&this->light_power, sizeof(this->light_power), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&this->light_range, sizeof(this->light_range), 1, file) != 1)
		return ERROR_FILE_READ;

	if (!this->TextureFromBuffer(buffer))
		return ERROR_FILE_READ;

	size_t iLayerCount;
	if (fread(&iLayerCount, sizeof(size_t), 1, file) != 1)
		return ERROR_FILE_READ;

	for (int i = 0; i < iLayerCount; ++i)
	{
		PsLayer* layer = new PsLayer;
		if (layer->FileLoad(file) != ERROR_NONE)
		{
			delete layer;
			return ERROR_FILE_WRITE;
		}
		this->aLayers.push_back(layer);
	}
	return ERROR_NONE;
}

/**
 * Save the pattern content to a file.
*/
ErrID PsPattern::FileSave(FILE* file) const
{
	uint8* buffer;
	size_t size;

	if (!(buffer = this->texture.GetBuffer(size)))
		return ERROR_FILE_WRITE;

	if (fwrite(&size, sizeof(size), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(buffer, sizeof(*buffer), size, file) != size)
		return ERROR_FILE_WRITE;

	if (fwrite(&this->hide, sizeof(this->hide), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&this->light_power, sizeof(this->light_power), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&this->light_range, sizeof(this->light_range), 1, file) != 1)
		return ERROR_FILE_WRITE;

	size = this->aLayers.size();
	if (fwrite(&size, sizeof(size_t), 1, file) != 1)
		return ERROR_FILE_WRITE;

	for (int i = 0; i < this->aLayers.size(); ++i)
		if (this->aLayers[i]->FileSave(file) != ERROR_NONE)
			return ERROR_FILE_WRITE;

	return ERROR_NONE;
}

/**
 * Get the name of this pattern (see PsShape::GetName)
*/
const std::string& PsPattern::GetName() const
{
	return this->name;
}

/**
 * Set the name of this pattern (see PsShape::SetName)
*/
void PsPattern::SetName(const std::string& name)
{
	this->name = name;
}

/**
 * Load a texture from a buffer (should be used when loading from a file).
*/
bool PsPattern::TextureFromBuffer(uint8* buffer)
{
	this->texture.LoadFromBuffer(buffer);
	this->ComputeLightMap();
	//  if (ColorIsSet())
	//	  SetRGB(color[0], color[1], color[2]);
	this->SetLinearLight(this->light_power, this->light_range);
	return true;
}

void BufferFlipVertical(uint8* buffer, int width, int height, int bpp)
{
	FIBITMAP* tmp = FreeImage_FromBuffer(buffer, width, height, bpp);
	memcpy(buffer, FreeImage_GetBits(tmp), width * height * bpp);
	FreeImage_Unload(tmp);
}

/**
 * Load a texture from a file.
*/
bool PsPattern::TextureFromFile(const char* file)
{
	TIFF* image;
	uint16 photo, bps, spp;
	uint32 width, height;
	tsize_t stripSize;
	unsigned long imageOffset, result;
	int stripMax, stripCount;
	uint8* buffer;
	unsigned long bufferSize;

	// Open the TIFF image
	if ((image = TIFFOpen(file, "r")) == NULL) {
		fprintf(stderr, "Could not open incoming image\n");
		return false;
	}

	// Check that it is of a type that we support
	if ((TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bps) == 0) || (bps != 8)) {
		fprintf(stderr, "Either undefined or unsupported number of bits per sample\n");
		return false;
	}

	if ((TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &spp) == 0) || (spp < 4)) {
		fprintf(stderr, "Either undefined or unsupported number of samples per pixel\n");
		return false;
	}

	// Read in the possibly multiple strips
	stripSize = TIFFStripSize(image);
	stripMax = TIFFNumberOfStrips(image);
	imageOffset = 0;

	bufferSize = TIFFNumberOfStrips(image) * stripSize;
	buffer = new uint8[bufferSize];

	for (stripCount = 0; stripCount < stripMax; stripCount++) {
		if ((result = TIFFReadEncodedStrip(image, stripCount,
			buffer + imageOffset,
			stripSize)) == -1) {
			fprintf(stderr, "Read error on input strip number %d\n", stripCount);
			return false;
		}

		imageOffset += result;
	}

	// Deal with photometric interpretations
	if (TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &photo) == 0) {
		fprintf(stderr, "PsImage has an undefined photometric interpretation\n");
		return false;
	}

	if (TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width) == 0) {
		fprintf(stderr, "PsImage does not define its width\n");
		return false;
	}

	if (TIFFGetField(image, TIFFTAG_IMAGELENGTH, &height) == 0) {
		fprintf(stderr, "PsImage does not define its height\n");
		return false;
	}

	TIFFClose(image);

	if (spp < 4)
		return false;

	//-- Extraction of the RGBA layer
	int bufferl = width * height;
	uint8* point = buffer;
	uint8* rgba_buffer = new uint8[width * height * 4];
	uint8* rgba_point = rgba_buffer;
	for (int ii = 0; ii < bufferl; ++ii)
	{

#ifndef __BIG_ENDIAN__
		rgba_point[1] = point[1];
		rgba_point[0] = point[2];
		rgba_point[2] = point[0];
#else
		rgba_point[0] = point[0];
		rgba_point[1] = point[1];
		rgba_point[2] = point[2];
#endif

		//-- selection of the alpha
		int alpha = -1;
		for (int ia = 0; ia < spp - 3; ia++)
			if (alpha == -1 || alpha < *(point + 3 + ia))
				alpha = *(point + 3 + ia);
		rgba_point[3] = (uint8)alpha;
		//--

		rgba_point += 4;
		point += spp;
	}
	BufferFlipVertical(rgba_buffer, width, height, 4);
	//--

	//-- Extraction of each additional layer  
	for (int cn = 0; cn < spp - 3; cn++)
	{
		PsLayer* layer = new PsLayer;

		//-- extraction of the layer
		uint8* point = buffer;
		uint8* channel = new uint8[bufferl];
		for (int ii = 0; ii < bufferl; ++ii)
		{
			channel[ii] = *(point + 3 + cn);
			point += spp;
		}
		//--

		layer->Register(width, height, channel);
		this->aLayers.push_back(layer);
	}
	//--

	if (!this->texture.RegisterAndSave(width, height, 4, rgba_buffer))
		return false;

	this->ComputeLightMap();

	return true;
}

int PsPattern::GetChannelsCount()
{
	return this->aLayers.size();
}

/*
** Transform the texture for OpenGL, resizing it to the nearest power of 2, but with a maximum size.
** The original image size is width*height, it is in "bpp * 8" mode (i.e: specify 3 for 24 bpp, 4 for 32 bpp),
** and "pixels" is the array of pixels that compose it.
*/
bool PsPattern::RegisterLightMap(int width, int height, uint8* pixels)
{
	uint8* buffer;
	int color[2];
	int h;
	int i;
	int j;
	int n;
	int w;
	int x;
	int y;
	int bpp = 2;
	int max_resol = 512;

	for (h = 1; h < height && h < max_resol; )
		h <<= 1;

	for (w = 1; w < width && w < max_resol; )
		w <<= 1;

	buffer = new uint8[w * h * 2];

	for (x = w; x--; )
	{
		for (y = h; y--; )
		{
			color[0] = 0;
			color[1] = 0;

			i = x * width / w;
			n = 0;

			do
			{
				j = y * height / h;
				do
				{
					color[0] += pixels[(i + j * width) * 2];
					color[1] += pixels[(i + j * width) * 2 + 1];
					++n;
				} while (++j < (y + 1) * height / h);
			} while (++i < (x + 1) * width / w);

			buffer[(x + y * w) * 2] = color[0] / n;
			buffer[(x + y * w) * 2 + 1] = color[1] / n;
		}
	}

	glGenTextures(1, &this->y_map_texture_id);
	glBindTexture(GL_TEXTURE_2D, this->y_map_texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	delete[] buffer;

	return true;
}

void PsPattern::ComputeLightMap()
{
	int bpp = 0;
	float Y_average = 0;
	uint32 total = 0;

	if (this->y_map_texture_data) delete this->y_map_texture_data;
	this->y_map_texture_data = new uint8[this->texture.height * this->texture.width * 2];

	uint8* data = this->texture.GetBufferUncompressed(bpp);
	uint8* pixels = data;
	uint8* ypixels = this->y_map_texture_data;
	for (uint32 i = 0; i < this->texture.width * this->texture.height; ++i)
	{
		ypixels[0] = 0; // set to black
		if (pixels[3] < this->minimumAlpha)
		{
			ypixels[1] = 0; // zero alpha on the pixel -> no processing
		}
		else
		{
			float Y = 0.299f * (pixels[0] / 255.f) + 0.587f * (pixels[1] / 255.f) + 0.114f * (pixels[2] / 255.f);
			Y = (Y > 1.f) ? 1.f : ((Y < 0.f) ? 0.f : Y);
			ypixels[1] = (1.f - Y) * 255; // save the inverted luminance in the alpha layer
			Y_average += Y;
			total++;
		}
		pixels += bpp;
		ypixels += 2;
	}

	Y_average /= (float)total;
	for (uint32 i = 0; i < this->texture.width * this->texture.height * 2; i += 2)
	{
		ypixels = this->y_map_texture_data + i;
		if (ypixels[1] != 0)
		{
			float Y = ((float)(255 - ypixels[1]) / 255.f) - Y_average; // centering on the average
			if (Y < 0) // if brightening
			{
				ypixels[0] = 255; // set to white
				Y = -Y; // invert the alpha
			}
			Y = (Y > 1.f) ? 1.f : ((Y < 0.f) ? 0.f : Y);
			ypixels[1] = Y * 255;
		}
	}

	this->RegisterLightMap(this->texture.width, this->texture.height, this->y_map_texture_data);
	delete data;
	data = 0;
}

void PsPattern::SetLinearLight(float linear_power, float linear_range)
{
	return;
	int bpp = 4;
	int pline = 0;
	float linear_crop = (1.f - linear_range) / 2;
	float linear_light = 0.f;
	float linear_step = linear_power / this->texture.height;
	uint8* pixels = 0;
	uint8* data = new uint8[this->texture.width * this->texture.height * bpp];
	memcpy(data, this->y_map_texture_data, this->texture.width * this->texture.height * bpp);
	for (uint32 j = 0; j < (uint32)this->texture.height; ++j)
	{
		pline = j * this->texture.width * bpp;
		if (j > this->texture.height * linear_crop
			&& j < this->texture.height * (1.f - linear_crop))
			linear_light += linear_step;
		for (uint32 i = 0; i < (uint32)this->texture.width; ++i)
		{
			pixels = data + pline + i * bpp;
		}
	}
	this->texture.Register(this->texture.width, this->texture.height, bpp, data);
	this->light_power = linear_power;
	this->light_range = linear_range;
	delete[] data;
}

int PsPattern::GetWidth()
{
	return this->texture.width;
}

int PsPattern::GetHeight()
{
	return this->texture.height;
}

void PsPattern::UpdateScale(int iDstWidth, int iDstHeight)
{
	this->fScaleWidth = (float)iDstWidth / (float)this->GetWidth();
	this->fScaleHeight = (float)iDstHeight / (float)this->GetHeight();
}
