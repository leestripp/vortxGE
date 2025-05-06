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
