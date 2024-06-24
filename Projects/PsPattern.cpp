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

float	PsPattern::minimumAlpha = 240;

PsPattern::PsPattern()
{
	/*
	 color[0] = -1;
	 color[1] = -1;
	 color[2] = -1;
	 */ 
	hide = false;
	light_power = 0.0f;
	light_range = 1.0f;
	y_map_texture_id = 0;
	y_map_texture_data = 0;
}

PsPattern::~PsPattern()
{
	if (y_map_texture_id)
	{
		glDeleteTextures (1, &y_map_texture_id);
		y_map_texture_id = 0;
	}
	if (y_map_texture_data)
	{
		delete[] y_map_texture_data;
		y_map_texture_data = 0;
	}
	for (int i = 0; i < aLayers.size(); ++i)
	{
		delete aLayers[i];
		aLayers[i] = 0;
	}
	aLayers.clear();
}

/**
 * Charge le contenu du patron depuis un fichier.
*/
ErrID	PsPattern::FileLoad (FILE* file)
{
	uint8*	buffer;
	size_t	size;
	
	if (fread (&size, sizeof (size), 1, file) != 1)
		return ERROR_FILE_READ;
	
	buffer = new uint8 [size];
	
	if (fread (buffer, sizeof (*buffer), size, file) != size)
		return ERROR_FILE_READ;
	
	if (fread (&hide, sizeof (hide), 1, file) != 1)
		return ERROR_FILE_READ;
	
	if (fread (&light_power, sizeof (light_power), 1, file) != 1)
		return ERROR_FILE_READ;
	
	if (fread (&light_range, sizeof (light_range), 1, file) != 1)
		return ERROR_FILE_READ;
	
	if (!TextureFromBuffer (buffer))
		return ERROR_FILE_READ;

	size_t iLayerCount;
	if (fread (&iLayerCount, sizeof (size_t), 1, file) != 1)
		return ERROR_FILE_READ;
	
	for (int i = 0; i < iLayerCount; ++i)
	{
		PsLayer *layer = new PsLayer;
		if (layer->FileLoad(file) != ERROR_NONE)
		{
			delete layer;
			return ERROR_FILE_WRITE;
		}
		aLayers.push_back(layer);
	}
	return ERROR_NONE;
}

/**
 * Charge le contenu d'un patron dans un fichier.
*/
ErrID	PsPattern::FileSave (FILE* file) const
{
	uint8*	buffer;
	size_t			size;
	
	if (!(buffer = texture.GetBuffer (size)))
		return ERROR_FILE_WRITE;
	
	if (fwrite (&size, sizeof (size), 1, file) != 1)
		return ERROR_FILE_WRITE;
	
	if (fwrite (buffer, sizeof (*buffer), size, file) != size)
		return ERROR_FILE_WRITE;
	
	if (fwrite (&hide, sizeof (hide), 1, file) != 1)
		return ERROR_FILE_WRITE;
	
	if (fwrite (&light_power, sizeof (light_power), 1, file) != 1)
		return ERROR_FILE_WRITE;
	
	if (fwrite (&light_range, sizeof (light_range), 1, file) != 1)
		return ERROR_FILE_WRITE;
	
	size = aLayers.size();
	if (fwrite (&size, sizeof (size_t), 1, file) != 1)
		return ERROR_FILE_WRITE;
	
	for (int i = 0; i < aLayers.size(); ++i)
		if (aLayers[i]->FileSave(file) != ERROR_NONE)
			return ERROR_FILE_WRITE;
	
	return ERROR_NONE;
}

/**
 * Récupere le nom de ce patron (voir PsShape::GetName)
*/
const std::string&	PsPattern::GetName() const
{
	return this->name;
}

/**
 * Change le nom de ce patron (voir PsShape::SetName)
*/
void	PsPattern::SetName (const std::string& name)
{
	this->name = name;
}

/**
 * Défini si une couleur a été attribuée
*/
/*
 bool  PsPattern::ColorIsSet()
 {
	 return color[0] != -1 &&  color[1] != -1 &&  color[2] != -1;
 }
 */

/**
 * Charge une texture depuis un buffer (doit être utilisé lors du chargement d'un fichier).
*/
bool	PsPattern::TextureFromBuffer (uint8* buffer)
{
	texture.LoadFromBuffer (buffer);
	ComputeLightMap();
	//  if (ColorIsSet())
	//	  SetRGB(color[0], color[1], color[2]);
	SetLinearLight(light_power, light_range);
	return true;
}

void BufferFlipVertical(uint8 *buffer, int width, int height, int bpp)
{
	FIBITMAP *tmp = FreeImage_FromBuffer(buffer, width, height, bpp);
	memcpy(buffer, FreeImage_GetBits(tmp), width * height * bpp);
	FreeImage_Unload(tmp);
}

/**
 * Charge une texture depuis un fichier.
*/
bool	PsPattern::TextureFromFile (const char* file)
{
	
	TIFF *image;
	uint16 photo, bps, spp;
	uint32 width, height;
	tsize_t stripSize;
	unsigned long imageOffset, result;
	int stripMax, stripCount;
	uint8 *buffer;
	unsigned long bufferSize;
	
	// Open the TIFF image
	if((image = TIFFOpen(file, "r")) == NULL){
		fprintf(stderr, "Could not open incoming image\n");
		return false;
	}
	
	// Check that it is of a type that we support
	if((TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bps) == 0) || (bps != 8)){
		fprintf(stderr, "Either undefined or unsupported number of bits per sample\n");
		return false;
	}
	
	if((TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &spp) == 0) || (spp < 4)){
		fprintf(stderr, "Either undefined or unsupported number of samples per pixel\n");
		return false;
	}
	
	// Read in the possibly multiple strips
	stripSize = TIFFStripSize (image);
	stripMax = TIFFNumberOfStrips (image);
	imageOffset = 0;
	
	bufferSize = TIFFNumberOfStrips (image) * stripSize;
	buffer = new uint8[bufferSize];
	
	for (stripCount = 0; stripCount < stripMax; stripCount++){
		if((result = TIFFReadEncodedStrip (image, stripCount,
										   buffer + imageOffset,
										   stripSize)) == -1){
			fprintf(stderr, "Read error on input strip number %d\n", stripCount);
			return false;
		}
		
		imageOffset += result;
	}
	
	// Deal with photometric interpretations
	if(TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &photo) == 0){
		fprintf(stderr, "PsImage has an undefined photometric interpretation\n");
		return false;
	}
	
	if(TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width) == 0){
		fprintf(stderr, "PsImage does not define its width\n");
		return false;
	}
	
	if(TIFFGetField(image, TIFFTAG_IMAGELENGTH, &height) == 0){
		fprintf(stderr, "PsImage does not define its height\n");
		return false;
	}
	
	TIFFClose(image);
	
	if (spp < 4)
		return false;
	
	//-- Extraction de la couche RGBA
	int bufferl = width * height;
	uint8 *point = buffer;
	uint8 *rgba_buffer = new uint8[width * height * 4];
	uint8 *rgba_point = rgba_buffer;
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
		
		//-- selection de l'alpha
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
	
	//-- Extraction de chaque couche supplémentaire  
	for (int cn = 0; cn < spp - 3; cn++)
	{
		PsLayer *layer = new PsLayer;
		
		//-- extraction de la couche
		uint8 *point = buffer;
		uint8 *channel = new uint8[width * height];
		for (int ii = 0; ii < bufferl; ++ii)
		{
			channel[ii] = *(point + 3 + cn);
			point += spp;
		}
		//--
		
		layer->Register(width, height, channel);
		aLayers.push_back(layer);
	}
	//--
	
	if (!texture.RegisterAndSave(width, height, 4, rgba_buffer))
		return false;
	
	ComputeLightMap();
	
	return true;
}

int PsPattern::GetChannelsCount()
{
	return aLayers.size();
}

/*
** Transforme la texture pour OpenGL, en la redimentionnant à la puissance de 2 la plus proche, mais avec une
** taille maximum. La taille de l'image originale est width*height, elle est en mode "bpp * 8" (i.e : indiquer
** 3 pour 24 bpp, 4 pour 32 bpp), et "pixels" est le tableau des pixels qui la composent.
*/
bool PsPattern::RegisterLightMap(int width, int height, uint8* pixels)
{
	uint8*			buffer;
	int				color[2];
	int				h;
	int				i;
	int				j;
	int				n;
	int				w;
	int				x;
	int				y;
	int				bpp = 2;
	int				max_resol = 512;

	for(h = 1; h < height && h < max_resol; )
		h <<= 1;

	for(w = 1; w < width && w < max_resol; )
		w <<= 1;

	buffer = new uint8 [w * h * 2];
	
	for(x = w; x--; )
	{
		for(y = h; y--; )
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
				}
				while(++j <(y + 1) * height / h);
			}
			while(++i <(x + 1) * width / w);

			buffer[(x + y * w) * 2] = color[0] / n;
			buffer[(x + y * w) * 2 + 1] = color[1] / n;
		}
	}
	
	glGenTextures(1, &y_map_texture_id);
	glBindTexture(GL_TEXTURE_2D, y_map_texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	delete [] buffer;

	return true;
}

void	PsPattern::ComputeLightMap()
{
	int bpp = 0;
	float Y_average = 0;
	uint32 total = 0;
	
	if (y_map_texture_data) delete y_map_texture_data; 
	y_map_texture_data = new uint8[texture.height * texture.width * 2];
	
	uint8 *data = texture.GetBufferUncompressed(bpp);
	uint8 *pixels = data;
	uint8 *ypixels = y_map_texture_data;
	for (uint32 i = 0; i < texture.width * texture.height; ++i)
	{
		ypixels[0] = 0; // on met du noir
		if (pixels[3] < minimumAlpha)
		{
			ypixels[1] = 0; // alpha nul sur le pixel -> pas de traitement
		}
		else
		{
			float Y = 0.299f * (pixels[0] / 255.f) + 0.587f * (pixels[1] / 255.f) + 0.114f * (pixels[2] / 255.f);
			Y = (Y>1.f)?1.f:((Y<0.f)?0.f:Y); 
			ypixels[1] = (1.f - Y) * 255; // enregistrement de la luminance inversée dans la couche apha
			Y_average += Y;
			total++;
		}
		pixels += bpp;
		ypixels += 2; 
	}

	Y_average /= (float)total;
	for (uint32 i = 0; i < texture.width * texture.height * 2; i += 2)
	{
		ypixels = y_map_texture_data + i;
		if (ypixels[1] != 0)
		{
			float Y = ((float)(255-ypixels[1]) / 255.f) - Y_average; // centrage sur la moyenne
			if (Y < 0) // si eclaircissement
			{
				ypixels[0] = 255; // on met du blanc
				Y = -Y; // on inverse l'alpha
			}
			Y = (Y>1.f)?1.f:((Y<0.f)?0.f:Y);
			ypixels[1] = Y * 255;
		}
	}
		
	RegisterLightMap(texture.width, texture.height, y_map_texture_data);
	delete data;
	data = 0;
}

/*
 void	PsPattern::SetRGB(uint8 R, uint8 G, uint8 B, PsPattern::MixType m)
 {
	 int bpp = 0;
	 int pline = 0;
	 uint8 *data = texture.GetBufferUncompressed(bpp);
	 uint8 *pixels = data;
	 
	 color[0] = R;
	 color[1] = G;
	 color[2] = B;
	 for (uint32 j = 0; j < (uint32)texture.height; ++j)
	 {
		 for (uint32 i = 0; i < (uint32)texture.width; ++i)
		 {
			 if (!(bpp == 4) || pixels[3] > minimumAlpha)
			 {
				 pixels[0] = B;
				 pixels[1] = G;
				 pixels[2] = R;
			 }
			 pixels += bpp;
		 }
	 }
	 texture.Register (texture.width, texture.height, bpp, data, false);
	 delete data;
	 data = 0;
 }
 */

/*
 void	PsPattern::SetYUV(float Y, float U, float V, PsPattern::MixType m)
 {
	 int bpp = 0;
	 int pline = 0;
	 float Y_prev = 0.f;
	 float y_start = 0.f;
	 if (m == Y_LIGHTER) y_start = y_first - Y_max;
	 else y_start = y_first - (float)y_average;
	 bool first_pixel = true;
	 uint8 *pixels = 0;
	 std::vector<float>::iterator y_var = y_map.begin();
	 uint8 *data = texture.GetBufferUncompressed(bpp);
	 for (uint32 j = 0; j < (uint32)texture.height; ++j)
	 {
		 pline = j * texture.width * bpp;
		 for (uint32 i = 0; i < (uint32)texture.width; ++i)
		 {
			 pixels = data + pline + i * bpp;
			 if (!(bpp == 4) || pixels[3] > minimumAlpha)
			 {
				 float Y_res = 0.f;
				 if (first_pixel)
				 {
					 Y_res = Y + y_start;
					 first_pixel = false;
				 }
				 else
				 {
					 Y_res = Y_prev + *y_var;
					 y_var++;
				 }
				 Y_prev = Y_res;
				 Y_res = (Y_res>1.f)?1.f:((Y_res<0.f)?0.f:Y_res); 
				 float r = (Y_res - 0.000039457070707f * V + 1.139827967171717f * U) * 255.f;
				 float g = (Y_res - 0.394610164141414f * V - 0.580500315656566f * U) * 255.f;
				 float b = (Y_res + 2.031999684343434f * V - 0.000481376262626f * U) * 255.f;
				 pixels[0] = (uint8)((r>255)?255:((r<0)?0:r));
				 pixels[1] = (uint8)((g>255)?255:((g<0)?0:g));
				 pixels[2] = (uint8)((b>255)?255:((b<0)?0:b));
			 }
		 }
	 }
	 texture.Register(texture.width, texture.height, bpp, data, false);
	 if (tweaked_data)
		 delete tweaked_data;
	 tweaked_data = data;
 }
 */

void	PsPattern::SetLinearLight(float linear_power, float linear_range)
{
	return;
	int bpp = 4;
	int pline = 0;
	float linear_crop = (1.f - linear_range) / 2;
	float linear_light = 0.f;
	float linear_step = linear_power / texture.height;
	uint8 *pixels = 0;
	uint8 *data = new uint8[texture.width * texture.height * bpp];
	memcpy(data, y_map_texture_data, texture.width * texture.height * bpp);
	for (uint32 j = 0; j < (uint32)texture.height; ++j)
	{
		pline = j * texture.width * bpp;
		if (j > texture.height * linear_crop 
			&& j < texture.height * (1.f - linear_crop))
			linear_light += linear_step;
		for (uint32 i = 0; i < (uint32)texture.width; ++i)
		{
			pixels = data + pline + i * bpp;
			if (!(bpp == 4) || pixels[3] > minimumAlpha)
			{
				/*float Y = 0.299f * (pixels[0] / 255.f) + 0.587f * (pixels[1] / 255.f) + 0.114f * (pixels[2] / 255.f);
				Y = (Y>1.f)?1.f:((Y<0.f)?0.f:Y); 
				float V = 0.492f * (pixels[2] / 255.f - Y); 
				float U = 0.877f * (pixels[0] / 255.f - Y);
				Y += linear_light;*/
				//if (Y > Y_max) Y = Y_max;
				/*Y = (Y>1.f)?1.f:((Y<0.f)?0.f:Y); 
				float r = (Y - 0.000039457070707f * V + 1.139827967171717f * U) * 255.f;
				float g = (Y - 0.394610164141414f * V - 0.580500315656566f * U) * 255.f;
				float b = (Y + 2.031999684343434f * V - 0.000481376262626f * U) * 255.f;
				pixels[0] = (uint8)((r>255)?255:((r<0)?0:r));
				pixels[1] = (uint8)((g>255)?255:((g<0)?0:g));
				pixels[2] = (uint8)((b>255)?255:((b<0)?0:b));*/
			}
		}
	}
	texture.Register(texture.width, texture.height, bpp, data);
	light_power = linear_power; 
	light_range = linear_range;
	delete[] data;
}

int PsPattern::GetWidth()
{
	return texture.width;
}

int PsPattern::GetHeight()
{
	return texture.height;
}

void PsPattern::UpdateScale(int iDstWidth, int iDstHeight)
{
	fScaleWidth = (float)iDstWidth / (float)GetWidth();
	fScaleHeight = (float)iDstHeight / (float)GetHeight();
}
