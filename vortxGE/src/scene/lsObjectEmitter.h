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

#ifndef LSOBJECTEMITTER_H
#define LSOBJECTEMITTER_H

#include <iostream>
#include <string>

// opengl
#include "../opengl/lsOpenGL.h"

// timers
#include "../timers/lsTimer.h"

// bullet
#include <bullet/btBulletDynamicsCommon.h>

// math
#include "../math/lsRandom.h"
#include "../math/lsVector3.h"

#include "lsAnimNode.h"

class lsAsset_Mesh;
class lsAsset;
class lsScene;
class lsLight;

using namespace std;


//! Emits an Object with physics.
/*!
 * Emits a mesh object set by the user.
 * Attach a physics shape to enable collisions.
 *
 * Remember to call Init_Animation() after creating an instance or no animation will be applied.
 *
 */
class lsObjectEmitter : public lsAnimNode
{
public:
    lsObjectEmitter( lsAsset_Mesh *am, lsAsset *abs, lsNode* parent );
    virtual ~lsObjectEmitter();

    // Emit
    void EmitObject( lsScene *scn, lsOpenGL *opengl );

    // Override functions
    virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );

    // Get Set
    double getEmittime() const;
    void setEmittime(double value);

    lsAsset_Mesh *getEmit_mesh() const;
    void setEmit_mesh(lsAsset_Mesh *value);

    lsAsset *getEmit_bulletshape() const;
    void setEmit_bulletshape(lsAsset *value);

    double getEmit_life() const;
    void setEmit_life(double value);

    lsLight *getEmit_light() const;
    void setEmit_light(lsLight *value);

    lsVector3 getRand_area() const;
    void setRand_area(const lsVector3 &value);

    lsVector3 getRand_velocity() const;
    void setRand_velocity(const lsVector3 &value);

    bool getRand_rotation() const;
    void setRand_rotation(bool value);

private:
    lsTimer         timer_emit;
    double          emittime;

    // Emit data
    lsScene         *m_scene;
    lsAsset_Mesh    *emit_mesh;
    lsAsset         *emit_bulletshape;
    // Life
    double          emit_life;
    // Light
    lsLight         *emit_light;

    // Random ranges
    lsRandom m_random;
    lsVector3   rand_area;
    lsVector3   rand_velocity;
    bool        rand_rotation;

};

#endif // LSOBJECTEMITTER_H
