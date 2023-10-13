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

// core
#include "../vortx_debug.h"

// math
#include "../math/lsVector3.h"

// scene
#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"

// passes
#include "../passes/lsPass.h"

// assets
#include "../assets/lsAsset.h"

// datablocks
#include "../datablocks/lsMesh.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector3.h"

// animation
#include "../animation/lsAnimationManager.h"

// lights
#include "../lights/lsLightManager.h"

// Physics
#include "../physics/lsBulletObject.h"

#include "lsObject.h"


lsObject::lsObject( lsNode* parent ) : lsAnimNode(parent)
{
    setNode_type( LSNT_OBJECT );
    setName("Object");

    asset = NULL;
    bulletobject = NULL;

    // Draw objects
    next_drawobject = NULL;
    prev_drawobject = NULL;
    // Culling
    cull_mesh = false;
    occluded = false;
    occlusion_type = LSOT_NONE;
    occlusion_id = 0;

    // Create light manager
    lightmanager = new lsLightManager();
    if(! lightmanager )
    {
        // ERROR
        cout << "lsObject::lsObject - ERROR: could now create lsLightManager instance" << endl;
    }
}

lsObject::~lsObject()
{
    if( asset ) asset->RemoveUser();
    if( lightmanager ) delete lightmanager;
    // bulletobjects are not managed so must be deleted.
    if( bulletobject ) delete bulletobject;
}


void lsObject::Move( lsScene* scn, lsOpenGL* opengl, double delta )
{
    if( bulletobject )
    {
        bulletobject->Apply_Physics( this );

    } else
    {
        // apply animation
        if( getAnimationManager() ) getAnimationManager()->UpdateAll( delta );
        if( asset )
        {
            if( asset->getData() )
            {
                lsMesh *mesh = NULL;
                switch( asset->getAsset_type() )
                {
                case LSAT_MESH:
                case LSAT_QUAD:
                    mesh = (lsMesh*)asset->getData();
                    // Not setup yet
                    mesh->Update_Animation();
                    break;

                }// switch data_type
            }
        }// if asset

        // Set transform
        lsMatrix4x4 transmat;
        lsMatrix4x4 rotmat;
        lsMatrix4x4 scalemat;

        transmat.TranslateMatrix( position.x, position.y, position.z );
        rotmat.RotateMatrix( rotation.x, 1, 0, 0 );
        rotmat.RotateMatrix( rotation.y, 0, 1, 0 );
        rotmat.RotateMatrix( rotation.z, 0, 0, 1 );
        scalemat.ScaleMatrix( scale.x, scale.y, scale.z );

        if( getParent() )
            ModelMatrix = getParent()->ModelMatrix;
        else
            ModelMatrix.LoadIdentity();

        ModelMatrix *= transmat * rotmat * scalemat;
    }

    // time to die
    checkLife( scn );
}

void lsObject::Optimize( lsCamera *cam, lsPass *pass, lsScene *scn, lsOpenGL *opengl )
{
    switch( pass->getPass_type() )
    {
    case LSPT_SHADOWMAP:
        // need to reset
        setOccluded( false );
        break;
    }

    // Culling
    setCull_mesh( true );
    if(( asset )&&( cam ))
    {
        // Calculate distance from camera
        lsVector3 objpos = lsVector3( ModelMatrix.matrix[12], ModelMatrix.matrix[13], ModelMatrix.matrix[14] );
        lsVector3 campos = lsVector3( cam->ModelMatrix.matrix[12], cam->ModelMatrix.matrix[13], cam->ModelMatrix.matrix[14] );
        setDistance_to_camera( campos.distance( objpos ) );

        // check frustum
        lsMesh *mesh;
        mesh = (lsMesh*)asset->getData();
        if( mesh )
        {
            int w = cam->frustum.sphereInFrustum( objpos + mesh->getBSphere().center, mesh->getBSphere().radius );
            if( w == INSIDE )
            {
                setCull_mesh( false );
            }
        }

        if( ! getcull_mesh() )
        {
            scn->Add_DrawObject( this );
        }
    }
}

void lsObject::Draw( lsCamera *cam, lsPass *pass, lsScene *scn, lsOpenGL *opengl )
{
    lsMesh *mesh = NULL;

    if( pass )
    {
        switch( pass->getPass_type() )
        {
        case LSPT_DIRECTRENDER:
        case LSPT_FBO:
            if(( asset )&&( ! getOccluded() ))
            {
                if( asset->getData() )
                {
                    switch( asset->getAsset_type() )
                    {
                    case LSAT_MESH:
                    case LSAT_QUAD:
                    case LSAT_MESHGUIDE_GRID:
                    case LSAT_MESHGUIDE_EMPTY:
                        mesh = (lsMesh*)asset->getData();
                        mesh->Draw( cam, pass, this );
                        break;

                    default:
                        cout << "lsObject::Draw - Unsupported asset type attached : " << asset->getAsset_type();
                        cout << " Name : " << asset->getAsset_name() << endl;
                        break;
                    }// switch
                }// if lsData
            }// if asset
            break;

        case LSPT_OCCLUSION:
            cout << "lsObject::Draw: Occlusion not supported yet" << endl;
            break;

        case LSPT_SHADOWMAP:
            cout << "lsObject::Draw: Shadow map not supported yet" << endl;
            break;

        default:
            cout << "lsObject::Draw: ERROR - Unknown pass type" << endl;
            break;

        }// switch pass_type
    }// if pass

    // Pass it on
    if( getNext_drawobject() ) getNext_drawobject()->Draw( cam, pass, scn, opengl );
}

void lsObject::DrawSingle( lsCamera *cam, lsPass *pass, lsScene *scn, lsOpenGL *opengl )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsObject::DrawSingle" << endl;
#endif

    lsMesh *mesh = NULL;

    if( pass )
    {
        switch( pass->getPass_type() )
        {
        case LSPT_OCCLUSION:
            if(( asset )&&( ! getOccluded() ))
            {
                if( asset->getData() )
                {
                    switch( asset->getAsset_type() )
                    {
                    case LSAT_MESHGUIDE_EMPTY:
                    case LSAT_MESHGUIDE_GRID:
                    case LSAT_MESH:
                    case LSAT_QUAD:
                        mesh = (lsMesh*)asset->getData();
                        mesh->Draw( cam, pass, this );
                        break;

                    default:
                        cout << "lsObject::DrawSingle - Unsupported asset type attached : " << asset->getAsset_type();
                        cout << " Name : " << asset->getAsset_name() << endl;
                        break;
                    }// switch
                }// if lsData
            }// if asset
            break;

        default:
            cout << "lsObject::DrawSingle: ERROR - Unknown pass type" << endl;
            break;

        }// switch pass_type
    }// if pass

    // DO NOT Pass it on
}

lsMesh* lsObject::Get_Mesh()
{
    if( ! asset ) return NULL;
    if( ! asset->getData() ) return NULL;

    switch( asset->getAsset_type() )
    {
    case LSAT_MESH:
        return (lsMesh *)asset->getData();
        break;

    }
    return NULL;
}

void lsObject::Set_Asset( lsAsset *am )
{
    if( am )
    {
        asset = am;
        am->AddUser();
    }
}

// Transforms

void lsObject::Set_Pos(float x, float y, float z)
{
    if( bulletobject )
    {
        bulletobject->Set_Position( x, y, z );
    } else
    {
        position = lsVector3( x, y, z );
    }
}

void lsObject::Set_Rot(float x, float y, float z)
{
    rotation = lsVector3( x, y, z );
}

void lsObject::Set_Scale(float x, float y, float z)
{
    scale = lsVector3( x, y, z );
}


// Save data
const string& lsObject::toString()
{
    save_str = "add_object ";
    if( getName() != "" )
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "`Object`";
    }

    if( getParent() )
    {
        save_str += " `";
        save_str += getParent()->getName();
        save_str += "`";
    } else
    {
        save_str += " `ERROR`";
    }

    // Pos, Rot and scale
    save_str += " ";
    save_str += position.toString();
    save_str += " ";
    save_str += rotation.toString();
    save_str += " ";
    save_str += scale.toString();

    // Asset name
    if( asset )
    {
        save_str += " `";
        save_str += asset->getAsset_name();
        save_str += "`";
    } else
    {
        save_str += " `NONE`";
    }

    return save_str;
}


