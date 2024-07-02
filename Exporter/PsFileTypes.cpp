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
#include "PsFileTypes.h"

char* READ_FORMATS[READ_FORMAT_COUNT][2] = {
{ "Tous les formats", "*.PSD;*.BMP;*.RLE;*.DIB;*.GIF;*.JPG;*.JPEG;*.JPE;*.PCX;*.PCD;*.PNG;*.TGA;*.TIF;*.TIFF"},
{ "Photoshop", "*.PSD" },
{ "TIFF", "*.TIF;*.TIFF"},
{ "Targa", "*.TGA" },
{ "PNG", "*.PNG"},
{ "JPEG", "*.JPG;*.JPEG;*.JPE"},
{ "BMP", "*.BMP;*.RLE;*.DIB" },
{ "Photo CD", "*.PCD"},
{ "CompuServe GIF", "*.GIF" },
{ "PCX", "*.PCX"}
};

char* WRITE_FORMATS[WRITE_FORMAT_COUNT][2] = {
	{ "TIFF", "*.TIF;*.TIFF"},
	{ "Targa", "*.TGA" },
	{ "PNG", "*.PNG"},
	{ "JPEG", "*.JPG;*.JPEG;*.JPE"},
	{ "BMP", "*.BMP;*.RLE;*.DIB" }
};