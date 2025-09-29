#pragma once

#include <iostream>
#include <string>

class lsScene;
class lsOpenGL;

using namespace std;


class lsSceneManager
{
public:
    lsSceneManager();
    virtual ~lsSceneManager();
	
	// Calls Move on all scenes.
    void Update( lsOpenGL* opengl, double delta );
	
    // Linked scenes
    void Add_Scene( lsScene *scene );
    lsScene* FindScene( const string& name );
	
    // Save and Load
	void SaveScenes( const string& path );
	bool LoadSceneFromFile( const string& filename, lsScene* targetScene );
	
    void LinkLightsInScene(lsScene* scene, int maxLightsPerObject = 3);
	
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
