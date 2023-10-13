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

#ifndef vortxGE_lsComposite_h
#define vortxGE_lsComposite_h

#include "lsControl.h"

class lsSceneManager;
class lsPassManager;
class lsAssetManager;
class lsOpenGL;

//! Layered renders
/**
 * Use textures (FBO) from passes to build final output. But really its up to you.
 *
 * Contains a Pass Manager, Scene Manager and Asset Manager.
 *
 * */

class lsComposite : public lsControl
{
public:
    lsComposite( lsOpenGL *opengl, lsControl* p );
    virtual ~lsComposite();

    int Init();

    void Update( double delta );
    void Render();
	void Resize( int x, int y, int width, int height  );

    // get set
    lsSceneManager* getSceneManager() const
    {
        return scenemanager;
    }

    lsPassManager* getPassManager() const
    {
        return passmanager;
    }

    lsAssetManager* getAssetManager() const
    {
        return assetmanager;
    }


private:
    lsSceneManager      *scenemanager;
    lsPassManager       *passmanager;
    lsAssetManager      *assetmanager;

};


#endif
