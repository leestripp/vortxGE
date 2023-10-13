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

#ifndef vortxGE_lsSceneManager_h
#define vortxGE_lsSceneManager_h

#include <iostream>
#include <string>

class lsScene;
class lsOpenGL;

using namespace std;

//! Scene Manager
/*!
 * Update() : Calls the first scene's Move() which inits a ModelMatrix.
 * Then it calls MoveThread() on each node in the heirarchy.
 *
 * You can link more Scenes to a Scene.
 *
 */
class lsSceneManager
{
public:
    lsSceneManager();
    virtual ~lsSceneManager();

	// Calls Move on all scenes.
    void Update( lsOpenGL* opengl, double delta );

    // Linked scenes
    void Add_Scene( lsScene *scene );

    // Save and Load
	void SaveScenes( const string& path );
	void LoadScene( const string& file );

    lsScene* getScene_list() const
    {
        return scene_list;
    }

    void setScene_list( lsScene* val )
    {
        scene_list = val;
    }

private:
    lsScene* scene_list;
};


#endif
