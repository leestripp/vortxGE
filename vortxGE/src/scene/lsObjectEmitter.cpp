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

// Physics
#include "../physics/lsBulletObject.h"
#include "../physics/lsBulletWorld.h"

// Assets
#include "../assets/lsAsset.h"

// Datablocks
#include "../datablocks/lsBulletShape.h"

// Animation
#include "../animation/lsAnimationManager.h"

// Scene
#include "lsScene.h"
#include "lsObject.h"
#include "lsLight.h"

// Lights
#include "../lights/lsLightManager.h"

#include "lsObjectEmitter.h"

lsObjectEmitter::lsObjectEmitter( lsAsset_Mesh *am, lsAsset *abs, lsNode* parent  ) : lsAnimNode(parent)
{
    setNode_type( LSNT_OBJECT_EMITTER );
    setName("Object Emitter");

    // object data to emit
    emit_mesh = am;
    emit_bulletshape = abs;
    // Life
    emit_life = 5000.0;
    // Light
    emit_light = NULL;

    // randome defaults.
    rand_area = lsVector3( 5, 5, 5 );
    rand_velocity = lsVector3( 0, 0, 0 );
    rand_rotation = true;

    // Emit
    timer_emit.start_Timer();
    emittime = 2000.0;

    // Get the root node
    m_scene = parent->Get_Scene();
}

lsObjectEmitter::~lsObjectEmitter()
{
}

void lsObjectEmitter::EmitObject( lsScene *scn, lsOpenGL *opengl, double delta )
{
    lsObject *object;
    lsBulletObject *bulletobject;
    lsBulletShape *bulletshape;
    float ox, oy, oz;

    if( timer_emit.get_ElapsedTime() > emittime )
    {
        // reset timer
        timer_emit.start_Timer();

        object = new lsObject( m_scene );
        if( object )
        {
            object->Set_Asset( (lsAsset*)emit_mesh );
            // set poition 12-14
            ox = ModelMatrix.matrix[12] + m_random.Rand_FloatRange( -rand_area.x, rand_area.x );
            oy = ModelMatrix.matrix[13] + m_random.Rand_FloatRange( -rand_area.y, rand_area.y );
            oz = ModelMatrix.matrix[14] + m_random.Rand_FloatRange( -rand_area.z, rand_area.z );
            object->Set_Pos( ox, oy, oz );
            // Set life
            object->Set_Life( emit_life );
            // Light
            object->getLightManager()->Add_Light( emit_light );

            // Add node
            if( m_scene )
            {
                m_scene->Add_Node( object );
            }

            // Get our physics shape
            switch( emit_bulletshape->getAsset_type() )
            {
            case LSAT_BULLETSHAPE_BOX:
            case LSAT_BULLETSHAPE_PILL:
            case LSAT_BULLETSHAPE_PLANE:
            case LSAT_BULLETSHAPE_SPHERE:
                // TODO: Add extra shapes, convex hull, concave mesh etc...
                bulletshape = (lsBulletShape*)emit_bulletshape->getData();
                break;

            default:
                cout << "ERROR: Wrong Asset attached as bullet shape to " << getName() << endl;
                break;
            }

            // Add lsBulletObject
            bulletobject = new lsBulletObject( object, bulletshape->getShape(), scn->getBulletworld()->getMax_proxies() );
            if( bulletobject )
            {
                bulletobject->Init( 1.0 ); // Mass 0 = static
                bulletobject->Set_Gravity( 0, -10, 0 );
                // match position
                bulletobject->Set_Position( ox, oy, oz );

                // rotation
                if( rand_rotation )
                {
                    ox = m_random.Rand_FloatRange( 0, 360.0 );
                    oy = m_random.Rand_FloatRange( 0, 360.0 );
                    oz = m_random.Rand_FloatRange( 0, 360.0 );
                    bulletobject->Set_Rotation( ox, oy, oz );
                    object->Set_Rot( ox, oy, oz );
                }
                // velocity
                ox = m_random.Rand_FloatRange( -rand_velocity.x, rand_velocity.x );
                oy = m_random.Rand_FloatRange( -rand_velocity.y, rand_velocity.y );
                oz = m_random.Rand_FloatRange( -rand_velocity.z, rand_velocity.z );
                bulletobject->Set_Velocity( ox, oy, oz );

            }// if( bulletobject )

            // Update object ModelMatrix to stop flickering.
            // Set transform
            lsMatrix4x4 transmat;
            lsMatrix4x4 rotmat;
            lsMatrix4x4 scalemat;

            transmat.TranslateMatrix( object->position.x, object->position.y, object->position.z );
            rotmat.RotateMatrix( object->rotation.x, 1, 0, 0 );
            rotmat.RotateMatrix( object->rotation.y, 0, 1, 0 );
            rotmat.RotateMatrix( object->rotation.z, 0, 0, 1 );
            scalemat.ScaleMatrix( object->scale.x, object->scale.y, object->scale.z );
            object->ModelMatrix = transmat * rotmat * scalemat;

        }// if object
    }// if emittime
}


void lsObjectEmitter::Move( lsScene *scn, lsOpenGL *opengl, double delta )
{
    // apply animation
    if( getAnimationManager() ) getAnimationManager()->UpdateAll( delta );

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

    // Emit Object
    EmitObject( scn, opengl, delta );

    // Life
    checkLife( scn );
}

// #################################################

double lsObjectEmitter::getEmittime() const
{
    return emittime;
}

void lsObjectEmitter::setEmittime(double value)
{
    emittime = value;
}

lsAsset_Mesh *lsObjectEmitter::getEmit_mesh() const
{
    return emit_mesh;
}

void lsObjectEmitter::setEmit_mesh(lsAsset_Mesh *value)
{
    emit_mesh = value;
}

lsAsset *lsObjectEmitter::getEmit_bulletshape() const
{
    return emit_bulletshape;
}

void lsObjectEmitter::setEmit_bulletshape(lsAsset *value)
{
    emit_bulletshape = value;
}

double lsObjectEmitter::getEmit_life() const
{
    return emit_life;
}

void lsObjectEmitter::setEmit_life(double value)
{
    emit_life = value;
}

lsLight *lsObjectEmitter::getEmit_light() const
{
    return emit_light;
}

void lsObjectEmitter::setEmit_light(lsLight *value)
{
    emit_light = value;
}

lsVector3 lsObjectEmitter::getRand_area() const
{
    return rand_area;
}

void lsObjectEmitter::setRand_area(const lsVector3 &value)
{
    rand_area = value;
}

lsVector3 lsObjectEmitter::getRand_velocity() const
{
    return rand_velocity;
}

void lsObjectEmitter::setRand_velocity(const lsVector3 &value)
{
    rand_velocity = value;
}

bool lsObjectEmitter::getRand_rotation() const
{
    return rand_rotation;
}

void lsObjectEmitter::setRand_rotation(bool value)
{
    rand_rotation = value;
}

