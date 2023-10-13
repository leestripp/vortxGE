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

#ifndef vortxGE_lsMesh_h
#define vortxGE_lsMesh_h

#include <iostream>
#include <string>

// math
#include "../math/lsBoundingSphere.h"

#include "lsData.h"

class lsBatch;
class lsObject;
class lsCamera;
class lsPass;
class lsOpenGL;
class lsAssetManager;
class lsShaderManager;


using namespace std;

/*!
 * \brief The lsMesh class
 *
 * Manage a list of lsBatch objects for rendering.\n
 *  Subclassed from lsData.\n
 *
 */
class lsMesh : public lsData
{
public:
    lsMesh( lsOpenGL *opengl );
    virtual ~lsMesh();

    void Draw( lsCamera *cam, lsPass *pass, lsObject *object );
    void Update_Animation();

    void LoadVBM( const string& filename, lsAssetManager *am, lsShaderManager *sm, bool debug=false );
    void Add_Batch( lsBatch* batch );

    const lsBoundingSphere& getBSphere()
    {
        return bSphere;
    }

    lsBatch* getBatch_list() const
    {
        return batch_list;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);

private:
    lsBatch* batch_list;

    // Overall around all batches : Needs implementing
    lsBoundingSphere bSphere;
    // opengl
    lsOpenGL *ogl;

};


#endif
