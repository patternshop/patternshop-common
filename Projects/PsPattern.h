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
#ifndef PS_PATTERN_H__

#define PS_PATTERN_H__

#include <string>
#include <vector>
#include "PsMessage.h"
#include "PsTexture.h"
#include "PsLayer.h"

class	PsRender;

/*
** Un patron
*/
class	PsPattern
{
	friend class 	PsRender;
	
public:
	typedef enum	{ Y_LIGHTER, Y_AVERAGE } MixType;
	
public:
	PsPattern();
	virtual ~PsPattern();
	
	virtual ErrID FileLoad (FILE*);
	virtual ErrID FileSave (FILE*) const;
	
	const std::string&	GetName() const;
	void SetName (const std::string&);
	const uint32 GetAutoGenId() { return texture.GetAutoGenId(); }
	
	bool TextureFromBuffer (uint8*);
	bool TextureFromFile (const char*);
	
	void SetLinearLight(float, float);
	void ComputeLightMap();
	bool RegisterLightMap(int, int, uint8*);
	
	int GetChannelsCount();
	int GetWidth();
	int GetHeight();
	void UpdateScale(int, int);
	
public:
	PsTexture texture;
	std::string name;
	bool hide;
	float light_power; 
	float light_range;
	GLuint y_map_texture_id;
	uint8 *y_map_texture_data;
	std::vector<PsLayer*> aLayers;
	static float minimumAlpha;
	float fScaleWidth, fScaleHeight;
};

#endif /* PS_PATTERN_H__ */

