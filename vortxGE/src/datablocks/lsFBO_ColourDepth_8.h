/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef LSFBO_COLOURDEPTH_8_H
#define LSFBO_COLOURDEPTH_8_H

#include "../opengl/lsOpenGL.h"

#include "lsFBO.h"


//! OpenGL Frame Buffer Object
/**
 If subclassing this please remember to Store and restore the default IOS framebuffer\n
 IOS uses its own framebuffer object to draw into and at runtime we dont know what it is, so store the current one and then restore it before and afer you Bind to your buffer.
 */
class lsFBO_ColourDepth_8 : public lsFBO
{
public:
	lsFBO_ColourDepth_8( double width, double height, lsOpenGL *opengl );
	virtual ~lsFBO_ColourDepth_8();

    // Form lsFBO
	void Enable();
	void ClearBuffers();

    // Data

	GLuint getTexture()
	{
		return texture;
	}

	GLuint getDepthTexture()
	{
		return depth_texture;
	}


private:
	GLuint texture;
	GLuint depth_texture;

};

#endif // LSFBO_COLOURDEPTH_8_H
