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

#ifndef LSFRUSTUM_H_
#define LSFRUSTUM_H_

#include "lsPlane.h"
#include "lsVector3.h"
#include "lsMatrix4x4.h"


enum FrustumSides {
	TOP = 0,
	BOTTOM,
	LEFT,
	RIGHT,
	NEARP,
	FARP
};

enum FrustumLoc {
    OUTSIDE,
    INTERSECT,
    INSIDE
};

//! Defines the viewing volume
/*!
 * Used for scene object culling.
 *
 */
class lsFrustum
{
public:
	lsFrustum();
	~lsFrustum();

	void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
	void Set_ViewRatio( float vr );;
	void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );

	void Build_CullPlanes( const lsVector3& p, const lsVector3& l, const lsVector3& u );

	// Check objects
	float distance( const lsVector3& V1 );
	int pointInFrustum( const lsVector3& p );
	int sphereInFrustum( const lsVector3& p, float raio );

	lsMatrix4x4& getProjectionMatrix()
	{
		return ProjectionMatrix;
	}

private:
	lsPlane		pl[6];

	float nearplane, farplane;
	float near_height, near_width;
	float far_height, far_width;
	float ratio, angle, tang;
	lsVector3 camerapos;

	// Projection
	lsMatrix4x4 ProjectionMatrix;

	// Points
	lsVector3 ntl, ntr, nbl, nbr;
	lsVector3 ftl, ftr, fbl, fbr;

};

#endif /* LSFRUSTUM_H_ */
