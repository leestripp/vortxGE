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

