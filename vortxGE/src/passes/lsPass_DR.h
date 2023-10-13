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

#ifndef vortxGE_lsPass_DR_h
#define vortxGE_lsPass_DR_h

#include "lsPass.h"


/** \brief Basic pass that uses the default system Framebuffer
 *
 * You bust setup a default GL framebuffer with at least Colour and Depth buffers.
 */

class lsPass_DR : public lsPass
{
public:
    lsPass_DR( lsOpenGL* opengl );
    virtual ~lsPass_DR();

    void Update();
    void Render();
	void Resize( int x, int y, int width, int height  );

    // Save
    virtual const string& toString();

private:

};


#endif
