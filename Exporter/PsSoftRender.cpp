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
#include "PsMaths.h"
#include "PsSoftRender.h"
#include "PsProject.h"
#include "FreeImage.h"
#include <map>

FIBITMAP* bigImage;
typedef std::map<PsImage*, FIBITMAP*> PrecalcImages;
PrecalcImages imageSoftwareMap;
bool freeImageInititialized = false;

//FREE_IMAGE_FILTER free_image_filter = FILTER_BSPLINE;
FREE_IMAGE_FILTER free_image_filter = FILTER_BICUBIC;

/*
** Dessine l'arrière plan (couleur de fond)
*/
void	DrawBackSoftwareFile()
{
	//-- sélection de la couleur de fond
	uint8 color[4];
	PsProjectController* project = PsController::Instance().project;
	if (!project->bHideColor)
	{
		color[0] = project->iColor[0];
		color[1] = project->iColor[1];
		color[2] = project->iColor[2];
		color[3] = 255;
	}
	else
	{
		color[0] = 255;
		color[1] = 255;
		color[2] = 255;
		color[3] = 0;
	}
	//--

	//-- application de la couleur de fond
	int bytespp = FreeImage_GetLine(bigImage) / FreeImage_GetWidth(bigImage);
	for (uint32 y = 0; y < FreeImage_GetHeight(bigImage); y++)
	{
		BYTE* bits = FreeImage_GetScanLine(bigImage, y);
		for (uint32 x = 0; x < FreeImage_GetWidth(bigImage); x++)
		{
			bits[FI_RGBA_RED] = color[0];
			bits[FI_RGBA_GREEN] = color[1];
			bits[FI_RGBA_BLUE] = color[2];
			bits[FI_RGBA_ALPHA] = color[3];
			bits += bytespp;
		}
	}
	//--
}

/*
** Demarre l'enregistrement en "Software" du rendu courant
*/
void InitSoftwareFile(int sx, int sy)
{
	if (!freeImageInititialized)
	{
		FreeImage_Initialise();
		freeImageInititialized = true;
	}

	bigImage = FreeImage_Allocate(sx, sy, 32);

	unsigned int pixelsPerMeter = round((double)PsController::Instance().project->GetDpi() / 0.0254);

	FreeImage_SetDotsPerMeterX(bigImage, pixelsPerMeter);
	FreeImage_SetDotsPerMeterY(bigImage, pixelsPerMeter);

	FreeImage_SetTransparent(bigImage, true);

	DrawBackSoftwareFile();
}

int getRotatedNorm(PsImage& image, int axe)
{
	int minimum = (int)image.corner[0][axe];
	for (int i = 1; i < 4; ++i)
	{
		if (image.corner[i][axe] < minimum)
			minimum = (int)image.corner[i][axe];
	}
	int maximum = (int)image.corner[0][axe];
	for (int i = 1; i < 4; ++i)
	{
		if (image.corner[i][axe] > maximum)
			maximum = (int)image.corner[i][axe];
	}
	return maximum - minimum;
}

/*
** Enregistre le patron dans l'image courante
*/
void PasteSoftwareFile(PsPattern& pattern/*, bool mask*/)
{
	//-- récupération des informations du patron
	int dsx = FreeImage_GetWidth(bigImage);
	int dsy = FreeImage_GetHeight(bigImage);
	int sx, sy, bpp = 4;
	pattern.texture.GetSize(sx, sy);
	uint8* realbuffer = pattern.texture.GetBufferUncompressed(bpp);
	//--

	//-- creation de l'image
	FIBITMAP* im = FreeImage_Allocate(sx, sy, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	FreeImage_SetTransparent(im, true);

	int bytespp = FreeImage_GetLine(im) / FreeImage_GetWidth(im);
	for (uint32 y = 0; y < FreeImage_GetHeight(im); y++)
	{
		BYTE* bits = FreeImage_GetScanLine(im, FreeImage_GetHeight(im) - y - 1);
		for (uint32 x = 0; x < FreeImage_GetWidth(im); x++)
		{
			uint8* location = realbuffer + (x + (y * (int)sx)) * bpp;
			if (bpp == 3)
			{
				bits[FI_RGBA_RED] = location[2];
				bits[FI_RGBA_GREEN] = location[1];
				bits[FI_RGBA_BLUE] = location[0];
				bits[FI_RGBA_ALPHA] = 255;
			}
			else
			{
				if (location[3] > PsPattern::minimumAlpha)
				{
					uint32 index = (y * sx + x) * 2;
					uint8 c = pattern.y_map_texture_data[index];
					bits[FI_RGBA_RED] = bits[FI_RGBA_GREEN] = bits[FI_RGBA_BLUE] = c;
					bits[FI_RGBA_ALPHA] = pattern.y_map_texture_data[index + 1];
				}
				else
				{
					bits[FI_RGBA_RED] = location[2];
					bits[FI_RGBA_GREEN] = location[1];
					bits[FI_RGBA_BLUE] = location[0];
					bits[FI_RGBA_ALPHA] = 255;
				}
			}
			bits += bytespp;
		}
	}
	delete[] realbuffer;

	FIBITMAP* cp = FreeImage_Rescale(im, dsx, dsy, FILTER_BSPLINE);
	FreeImage_BlendPaste(bigImage, cp, 0, 0);
	FreeImage_Unload(cp);
	FreeImage_Unload(im);
}

/*
** Enregistre l'image dans l'image courante
*/
void PasteSoftwareFile(PsImage& image, int x, int y)
{
	FIBITMAP* im = imageSoftwareMap[&image];

	if (!im)
	{
		int dsx = (int)abs(image.w);
		int dsy = (int)abs(image.h);

		// transormation de l'image dans le format demandé
		int sx, sy, bpp;
		image.GetTexture().GetSize(sx, sy);
		uint8* realbuffer = image.GetTexture().GetBufferUncompressed(bpp);
		im = FreeImage_FromBuffer(realbuffer, sx, sy, bpp);
		delete[] realbuffer;
		//--

		// gestion des dimensions "negatives" i.e. effet mirroir
		if (image.w < 0) FreeImage_FlipHorizontal(im);
		if (image.h < 0) FreeImage_FlipVertical(im);
		//--

		// redimensionement
		FIBITMAP* resim = FreeImage_Rescale(im, dsx, dsy, FILTER_BSPLINE);
		FreeImage_SetTransparent(resim, true);
		FreeImage_Unload(im);
		im = resim;
		//--

		// rotation
		if (image.GetAngle())
		{
			FIBITMAP* rotated = FreeImage_RotateClassic(im, image.GetAngle() * 180.0f / 3.14159265f);
			FreeImage_SetTransparent(rotated, true);
			FreeImage_Unload(im);
			im = rotated;
		}

		// map de pre-calcul
		imageSoftwareMap[&image] = im;
	}

	//-- finalement, copie
	float a, b;
	image.GetPosition(a, b);
	PsRender& renderer = PsController::Instance().project->renderer;
	FreeImage_BlendPaste(bigImage, im, (int)(x + a) - renderer.x1 - FreeImage_GetWidth(im) / 2, (int)(y + b) - renderer.y2 - FreeImage_GetHeight(im) / 2);
	//--

}

/*
** Fonction utilitaire de controle des extensions
*/
static bool FileCheckExtention(const char* path, const char* ext)
{
	char* mpath = strdup(path);
	mpath = strupr(mpath);
	char* mext = strdup(ext);
	mext = strupr(mext);
	bool result = (strstr(mpath, mext) == (char*)(mpath + (strlen(path) - strlen(ext))));
	delete mpath;
	delete mext;
	return result;
}

/*
** Creation d'un buffer à partir d'un fichier
*/
bool FreeImage_BufferFromFile(const char* path, uint8*& realbuffer, int& sx, int& sy, int& bpp)
{
	FREE_IMAGE_FORMAT frmt = FIF_UNKNOWN;

	if (FileCheckExtention(path, ".bmp")) frmt = FIF_BMP;
	else if (FileCheckExtention(path, ".rle")) frmt = FIF_BMP;
	else if (FileCheckExtention(path, ".dib")) frmt = FIF_BMP;
	else if (FileCheckExtention(path, ".cut")) frmt = FIF_CUT;
	else if (FileCheckExtention(path, ".dds")) frmt = FIF_DDS;
	else if (FileCheckExtention(path, ".g3")) frmt = FIF_FAXG3;
	else if (FileCheckExtention(path, ".gif")) frmt = FIF_GIF;
	else if (FileCheckExtention(path, ".hdr")) frmt = FIF_HDR;
	else if (FileCheckExtention(path, ".ico")) frmt = FIF_ICO;
	else if (FileCheckExtention(path, ".iff")) frmt = FIF_IFF;
	else if (FileCheckExtention(path, ".lbm")) frmt = FIF_IFF;
	else if (FileCheckExtention(path, ".jng")) frmt = FIF_JNG;
	else if (FileCheckExtention(path, ".jpg")) frmt = FIF_JPEG;
	else if (FileCheckExtention(path, ".jif")) frmt = FIF_JPEG;
	else if (FileCheckExtention(path, ".jpeg")) frmt = FIF_JPEG;
	else if (FileCheckExtention(path, ".jpe")) frmt = FIF_JPEG;
	else if (FileCheckExtention(path, ".koa")) frmt = FIF_KOALA;
	else if (FileCheckExtention(path, ".mng")) frmt = FIF_MNG;
	else if (FileCheckExtention(path, ".pbm")) frmt = FIF_PBMRAW;
	else if (FileCheckExtention(path, ".pcd")) frmt = FIF_PCD;
	else if (FileCheckExtention(path, ".pcx")) frmt = FIF_PCX;
	else if (FileCheckExtention(path, ".png")) frmt = FIF_PNG;
	else if (FileCheckExtention(path, ".ppm")) frmt = FIF_PPMRAW;
	else if (FileCheckExtention(path, ".psd")) frmt = FIF_PSD;
	else if (FileCheckExtention(path, ".ras")) frmt = FIF_RAS;
	else if (FileCheckExtention(path, ".sgi")) frmt = FIF_SGI;
	else if (FileCheckExtention(path, ".tga")) frmt = FIF_TARGA;
	else if (FileCheckExtention(path, ".targa")) frmt = FIF_TARGA;
	else if (FileCheckExtention(path, ".tif")) frmt = FIF_TIFF;
	else if (FileCheckExtention(path, ".tiff")) frmt = FIF_TIFF;
	else if (FileCheckExtention(path, ".xbm")) frmt = FIF_XBM;
	else if (FileCheckExtention(path, ".xpm")) frmt = FIF_XPM;

	// Vérification sur l'extension bête et méchante
	if (frmt == FIF_UNKNOWN) return false;

	if (!freeImageInititialized)
	{
		FreeImage_Initialise();
		freeImageInititialized = true;
	}

	//-- chargement en RGBA
	FIBITMAP* im = FreeImage_Load(frmt, path);
	FIBITMAP* rgba = FreeImage_ConvertTo32Bits(im);
	FreeImage_Unload(im);
	im = rgba;
	if (!im) return false;
	//--

	realbuffer = (uint8*)FreeImage_GetBits(im);
	sx = FreeImage_GetWidth(im);
	sy = FreeImage_GetHeight(im);
	bpp = FreeImage_GetBPP(im) / 8;

	return true;
}

/*
** Enregistre le rendu courant dans un fichier
*/
void flushSoftwareFile(const char* filename, bool warning_alpha)
{
	FREE_IMAGE_FORMAT frmt = FIF_UNKNOWN;

	if (FileCheckExtention(filename, ".bmp")) frmt = FIF_BMP;
	else if (FileCheckExtention(filename, ".rle")) frmt = FIF_BMP;
	else if (FileCheckExtention(filename, ".dib")) frmt = FIF_BMP;
	else if (FileCheckExtention(filename, ".jpg")) frmt = FIF_JPEG;
	else if (FileCheckExtention(filename, ".jif")) frmt = FIF_JPEG;
	else if (FileCheckExtention(filename, ".jpeg")) frmt = FIF_JPEG;
	else if (FileCheckExtention(filename, ".jpe")) frmt = FIF_JPEG;
	else if (FileCheckExtention(filename, ".png")) frmt = FIF_PNG;
	else if (FileCheckExtention(filename, ".tga")) frmt = FIF_TARGA;
	else if (FileCheckExtention(filename, ".targa")) frmt = FIF_TARGA;
	else if (FileCheckExtention(filename, ".tif")) frmt = FIF_TIFF;
	else if (FileCheckExtention(filename, ".tiff")) frmt = FIF_TIFF;

	// Vérification sur l'extension bête et méchante
	if (frmt == FIF_UNKNOWN) return; // FIXME : message d'erreur

	if (!freeImageInititialized)
	{
		FreeImage_Initialise();
		freeImageInititialized = true;
	}

	FreeImage_FlipVertical(bigImage);
	FreeImage_SetTransparent(bigImage, true);

	if (frmt == FIF_JPEG)
	{
		if (warning_alpha && !GetQuestion(QUESTION_LOOSE_ALPHA))
			return;
		FIBITMAP* bk = FreeImage_ConvertTo24Bits(bigImage);
		FreeImage_Unload(bigImage);
		bigImage = bk;
	}

	FreeImage_Save(frmt, bigImage, filename);

	//-- libération de la mémorie
	FreeImage_Unload(bigImage);
	PrecalcImages::iterator i;
	for (i = imageSoftwareMap.begin(); i != imageSoftwareMap.end(); ++i)
		FreeImage_Unload((*i).second);
	imageSoftwareMap.clear();
	//--

}


/*
** Déformation de l'image dans le quatrangle donné
*/
FIBITMAP* DrawSoftwareMapping(PsLayer* layer, FIBITMAP* src_img)
{
	PsProjectController* project = PsController::Instance().project;
	PsRender& renderer = PsController::Instance().project->renderer;

	int iWidth = FreeImage_GetWidth(src_img);
	int iHeight = FreeImage_GetHeight(src_img);

	//-- HACK : rendu en opengl (n'utilise même pas 'src_img' en haute résolution)
	int iTextureSize = renderer.iLayerTextureSize;
	uint8* result_data = new uint8[iTextureSize * iTextureSize * 4];
	int iOldLayerTextureSize = renderer.iLayerTextureSize;
	renderer.iLayerTextureSize = iTextureSize;
	renderer.engine = PsRender::ENGINE_HARDWARE;
	GLuint iDocTexture = renderer.CreateDocumentTexture(*project);
	renderer.engine = PsRender::ENGINE_SOFTWARE;
	renderer.UpdateLayerTexture(*project, layer, iDocTexture);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, layer->iFinalTextureId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, result_data);
	glDisable(GL_TEXTURE_2D);
	renderer.iLayerTextureSize = iOldLayerTextureSize;
	FIBITMAP* conv = FreeImage_FromBuffer(result_data, iTextureSize, iTextureSize, 4);

	int bytespp = FreeImage_GetLine(conv) / FreeImage_GetWidth(conv);
	for (uint32 y = 0; y < FreeImage_GetHeight(conv); y++)
	{
		BYTE* bits = FreeImage_GetScanLine(conv, FreeImage_GetHeight(conv) - y - 1);
		for (uint32 x = 0; x < FreeImage_GetWidth(conv); x++)
		{
			uint8 swap = bits[FI_RGBA_RED];
			bits[FI_RGBA_RED] = bits[FI_RGBA_BLUE];
			bits[FI_RGBA_BLUE] = swap;
			bits += bytespp;
		}
	}

	FIBITMAP* dst_img = FreeImage_Rescale(conv, iWidth, iHeight, FILTER_BSPLINE);
	FreeImage_Unload(conv);
	glDeleteTextures(1, &iDocTexture);
	glDeleteTextures(1, &layer->iFinalTextureId);
	layer->iFinalTextureId = NULL;
	//--

	return dst_img;
}

void DrawLayerSoftwareFile(int iIndex)
{
	//-- récupération des paramètres du projet
	PsProjectController* project = PsController::Instance().project;
	if (!project) return;
	if (!project->pattern) return;
	if (project->pattern->aLayers.size() <= iIndex) return;
	PsLayer* layer = project->pattern->aLayers[iIndex];
	PsRender& renderer = PsController::Instance().project->renderer;
	PsPattern* pattern = project->pattern;
	//--

	int iWidth = FreeImage_GetWidth(bigImage);
	int iHeight = FreeImage_GetHeight(bigImage);

	//-- création du buffer
	FIBITMAP* buffer = FreeImage_Allocate(iWidth, iHeight, 32);
	FreeImage_SetTransparent(buffer, true);
	//--

	//-- dessin des matrices
	/* FIXME : OpenGL HACK
	FIBITMAP *bigImageBackup = bigImage;
	bigImage = buffer;
	renderer.DrawMatrices(*project);
	bigImage = bigImageBackup;
	*/
	//--

	//-- application de la transformation
	FIBITMAP* transformed = DrawSoftwareMapping(layer, buffer);
	FreeImage_Unload(buffer);
	buffer = transformed;
	//--

	//-- application du masque alpha
	int bytespp = FreeImage_GetLine(buffer) / FreeImage_GetWidth(buffer);
	for (uint32 y = 0; y < FreeImage_GetHeight(buffer); y++)
	{
		BYTE* bits = FreeImage_GetScanLine(buffer, y);
		int _y = (int)((float)y / (float)pattern->fScaleHeight);
		for (uint32 x = 0; x < FreeImage_GetWidth(buffer); x++)
		{
			int _x = (int)((float)x / (float)pattern->fScaleWidth);
			uint8 alpha = layer->ucData[_y * pattern->texture.width + _x];
			if (alpha <= PsPattern::minimumAlpha)
			{
				bits[FI_RGBA_RED] = 0;
				bits[FI_RGBA_GREEN] = 0;
				bits[FI_RGBA_BLUE] = 0;
				bits[FI_RGBA_ALPHA] = 0;
			}
			bits += bytespp;
		}
	}
	//--

	//-- finalement, copie
	FreeImage_BlendPaste(bigImage, buffer, 0, 0);
	//--

	FreeImage_Unload(buffer);
}