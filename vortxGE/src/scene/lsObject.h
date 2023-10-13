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

#ifndef vortxGE_lsObject_h
#define vortxGE_lsObject_h

// opengl
#include "../opengl/lsOpenGL.h"

// scene
#include "../scene/lsAnimNode.h"

// Occlusion types;
const int LSOT_NONE         =	0;
const int LSOT_OCCLUDER		=	1;

class lsAsset;
class lsMesh;
class lsBulletObject;
class lsProperty;
class lsLightManager;
class lsAnimationManager;


/** \brief Holds pointers to an Asset (.vbm mesh), Light Manager and Bullet Object.
 *
 * Remember to call "Init_Animation( double fps )" after creating an instance or no animation will be applied.
 */

class lsObject : public lsAnimNode
{
public:
    lsObject( lsNode* parent );
    virtual ~lsObject();

    virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
    virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );

    // Occlusion
    void DrawSingle( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );

    // Transforms : override lsNode
    void Set_Pos(float x, float y, float z);
    void Set_Rot(float x, float y, float z);
    void Set_Scale(float x, float y, float z);

    lsMesh* Get_Mesh();

    // Save data
    virtual const string& toString();

    void Set_Asset( lsAsset* asset );
    lsAsset* getAsset() const
    {
        return asset;
    }

    lsBulletObject* getBulletObject() const
    {
        return bulletobject;
    }

    void setBulletObject( lsBulletObject* val )
    {
        bulletobject = val;
    }

    lsLightManager* getLightManager() const
    {
        return lightmanager;
    }

    // rendering
    bool getcull_mesh() const
    {
        return cull_mesh;
    }

    void setCull_mesh( bool val )
    {
        cull_mesh = val;
    }

    bool getOccluded() const
    {
        return occluded;
    }

    void setOccluded( bool val )
    {
        occluded = val;
    }

    float getDistance_to_camera() const
    {
        return distance_to_camera;
    }

    void setDistance_to_camera( float val )
    {
        distance_to_camera = val;
    }

    int getOcclusion_type() const
    {
        return occlusion_type;
    }

    void setOcclusion_type( int val )
    {
        occlusion_type = val;
    }

    GLuint getOcclusionID() const
    {
        return occlusion_id;
    }

    void setOcclusionID( GLuint val )
    {
        occlusion_id = val;
    }


    // Draw --------------------------

    lsObject* getNext_drawobject() const
    {
        return next_drawobject;
    }

    void setNext_drawobject( lsObject* val )
    {
        next_drawobject = val;
    }

    lsObject* getPrev_drawobject() const
    {
        return prev_drawobject;
    }

    void setPrev_drawobject( lsObject* val )
    {
        prev_drawobject = val;
    }


private:
    lsAsset             *asset;
    lsLightManager      *lightmanager;
    lsBulletObject      *bulletobject;

    // rendering draw list
    lsObject *next_drawobject;
    lsObject *prev_drawobject;

    // Culling
    bool cull_mesh;
    bool occluded;
    float distance_to_camera;
    int occlusion_type;
    GLuint occlusion_id;

};


#endif
