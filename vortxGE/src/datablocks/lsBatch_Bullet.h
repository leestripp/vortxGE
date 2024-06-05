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

#ifndef vortxGE_lsBatch_Bullet_h
#define vortxGE_lsBatch_Bullet_h

#include "lsBatch.h"
#include "../opengl/lsOpenGL.h"

// mmath
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"


class btBvhTriangleMeshShape;
class btConvexHullShape;


//! Bullet Mesh
/*!
 * Used to create a Convex Hull or Concave Triangle Mesh for use with the bullet physics engine.
 *
 * */
class lsBatch_Bullet : public lsBatch
{
public:
	lsBatch_Bullet( lsOpenGL *opengl );
	virtual ~lsBatch_Bullet();

    // lsBatch
    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

	void Begin( GLuint nMaxVerts );
	void AddTriangle( Vector3f verts[3] );
	btBvhTriangleMeshShape*  End_TriMesh( float size );
	btConvexHullShape*  End_ConvexHull();
	void Cleanup();

    // openGL data - on video card : needed by the asset system
    void Copy_GLdata();
    void Clear_GLdata();
	
	// Utils
	bool CloseEnough( const float fCandidate, const float fCompare, const float fEpsilon );
	
private:
	GLuint nMaxIndexes;
	GLuint nNumIndexes;
	GLuint nNumVerts;
	
	GLushort *pIndexes;
	Vector3f *pVerts;
};


#endif
