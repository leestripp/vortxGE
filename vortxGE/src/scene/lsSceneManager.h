#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

class lsScene;
class lsRender;

using namespace std;


class lsSceneManager
{
public:
	lsSceneManager();
	virtual ~lsSceneManager();
	
	// Calls Move on all scenes.
	void Update( lsRender* render, double delta );
	
	// Linked scenes
	void Add_Scene( lsScene* scene );
	lsScene* FindScene( const string& name );
	
	// Save and Load
	void SaveScenes( const string& path );
	bool LoadSceneFromFile( const string& filename, lsScene* targetScene );

	/// Live-link: re-apply transforms + light properties from a .vgs
	/// file to EXISTING nodes (matched by name). No structural changes
	/// (no add/remove) — safe for a running pipeline (no dangling
	/// light pointers). Returns the number of nodes updated.
	int ApplySceneTransforms( const string& filename, lsScene* targetScene );
	
	void LinkLightsInScene(lsScene* scene, int maxLightsPerObject = 3);
	
private:
	// Removed: lsScene *scene_list; // This was replaced by m_scenes
	vector<unique_ptr<lsScene>> m_scenes; // New member to manage scenes
};
