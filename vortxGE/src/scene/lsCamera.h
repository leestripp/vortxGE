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

#ifndef vortxGE_lsCamera_h
#define vortxGE_lsCamera_h

// math
#include "../math/lsFrustum.h"

// scene
#include "../scene/lsAnimNode.h"

const int LSCM_PERSPECTIVE      = 0;
const int LSCM_ORTHOGRAPHIC     = 1;

//! A scene camera
/*!
 * Used to view 3D objects. Yep!
 *
 */
class lsCamera : public lsAnimNode
{
public:
    lsCamera( lsNode* parent );
    virtual ~lsCamera();

    // from lsNode
    virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
    virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );

	void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
    void Set_ViewRatio( float vr );
	void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
    void Set_Orthographic( float cube );
    void Update_Camera();

    // Save data
    virtual const string& toString();

    int getCamera_mode() const
    {
        return camera_mode;
    }

    void setCamera_mode( int val )
    {
        camera_mode = val;
    }

    // Public data
    lsMatrix4x4 ViewMatrix;
    lsFrustum frustum;

private:
	int camera_mode;
    float cam_fov;
    float cam_ratio;
    float cam_near;
    float cam_far;
    float cam_orthoCube;
    // Auto update
    int update_interval;
    int update_counter;

};

#endif
