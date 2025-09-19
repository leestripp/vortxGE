
#pragma once

#include <thread>
#include <mutex>

#include "lsNode.h"

class lsBulletWorld;
class lsEvent;
class lsObject;
class lsAssetManager;
class lsLightManager;


using namespace std;


class lsScene : public lsNode
{
public:
    lsScene( lsOpenGL* ogl, float world_size, int maxProxies );
    virtual ~lsScene();
	
    // from lsNode
    void Move( lsScene* scn, lsOpenGL* opengl, double delta );
    void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    void InitDrawlist();
	
    // Get the base Scene, every node has this func.
    lsScene *Get_Scene();
    lsEventManager *Get_EventManager();
	
    void Del_Node( lsNode* node );
    void ClearScene();
    void Add_DrawObject( lsObject* object );
	
    // Events
    void Add_Event( lsEvent* event );
    lsEvent* Poll_Event();

    // Save data
    virtual const string& toString();
	
	
    lsObject* getDraw_list() const
    {
        return draw_list;
    }
	
    lsNode* getFound_node() const
    {
        return found_node;
    }
	
    void setFound_node( lsNode* val )
    {
        found_node = val;
    }

    // physics

    lsBulletWorld* getBulletworld() const
    {
        return bulletworld;
    }

    void setBulletworld( lsBulletWorld *val )
    {
        bulletworld = val;
    }

    lsAssetManager* getAssetManager() const
    {
        return m_assetManager;
    }

    lsLightManager* getLightManager() const
    {
        return m_lightManager;
    }
	
private:
    lsObject* draw_list;

    // Events
    lsEventManager *root_eventmanager;
    // physics
    lsBulletWorld   *bulletworld;
    // Assets
    lsAssetManager* m_assetManager;
    // Lights
    lsLightManager* m_lightManager;
    // Found node
    lsNode* found_node;
	
    // Threads
    mutex m_mutex;
};
