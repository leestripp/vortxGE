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

#include "../vortx_debug.h"

// OpenGL
#include "../opengl/lsOpenGL.h"

// Scene
#include "../scene/lsScene.h"

// datablocks
#include "../datablocks/lsStats.h"


#include "lsSceneManager.h"

lsSceneManager::lsSceneManager()
{
    scene_list = NULL;
}

lsSceneManager::~lsSceneManager()
{
    // Delete all scenes
    lsScene *scene, *nextscene;

    scene = scene_list;
    while( scene )
    {
        nextscene = (lsScene*)scene->getNext_node();

        // We must clear the scene before deletion
        scene->ClearScene();
        delete scene;

        scene = nextscene;
    }
    scene_list = NULL;
}

/*!
 * Move all scene objects
 */
void lsSceneManager::Update( lsOpenGL* opengl, double delta )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsSceneManager::Update" << endl;
#endif
    lsScene *scene = scene_list;

    // Move stats
    if( opengl->getStats() ) opengl->getStats()->Move_Start();
    while( scene )
    {
        // Init base matrix
        scene->Move( scene, opengl, delta );

        // Move nodes threaded
        scene->MoveThread( scene, opengl, delta );

        // Next scene
        scene = (lsScene*)scene->getNext_node();
    }
    if( opengl->getStats() ) opengl->getStats()->Move_Stop();
}

void lsSceneManager::Add_Scene( lsScene *scene )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsSceneManager::Add_Scene : " << scene->getName() << endl;
#endif

    if( scene_list ) scene_list->setPrev_node( scene );
    scene->setNext_node( scene_list );
    scene_list = scene;
}

// ################################
// Save and Load

void lsSceneManager::SaveScenes( const string& path )
{
	cout << "Save scenes to path : " << path << endl;
	lsScene *scene = scene_list;

    while( scene )
    {
		// open a file per scene

    	// pass file on
    	scene->Save();

        // Next scene
        scene = (lsScene*)scene->getNext_node();
    }
}

void lsSceneManager::LoadScene( const string& file )
{
	cout << "Load scene from file : " << file << endl;

	// create new scene

	// load nodes

	// Add to scene list

}

