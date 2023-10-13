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

#ifndef vortxGE_lsBulletShape_h
#define vortxGE_lsBulletShape_h

// opengl
#include "../opengl/lsOpenGL.h"

#include "lsData.h"

class btCollisionShape;

/**
 Holds the basic Bullet Shape to be attached to a lsBulletObject used by lsObjects
 */
class lsBulletShape : public lsData
{
public:
    lsBulletShape( lsOpenGL *opengl );
    virtual ~lsBulletShape();
    // opengl
    lsOpenGL *ogl;

	// build shapes
	void Make_Plane( float x, float y, float z );
	void Make_Box( float x, float y, float z );
	void Make_Sphere( float r );
	// From vbm file via lsBatch_Buller
	void Make_ConvexHull( const string& filename, lsOpenGL *opengl );
    void Make_TriMesh( const string& filename, float size = 1.0 );


	btCollisionShape *getShape() const
	{
		return shape;
	}

private:
    btCollisionShape *shape;

};


#endif
