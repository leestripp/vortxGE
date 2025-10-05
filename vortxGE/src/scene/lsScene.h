#pragma once

#include <thread>
#include <mutex>

#include "lsNode.h"

class lsBulletWorld;
class lsEvent;
class lsObject;
class lsAssetManager;
class lsLightManager;
class lsListener; // Forward declaration for lsListener

using namespace std;


class lsScene : public lsNode
{
public:
    lsScene( lsRender* m_render, float world_size, int maxProxies );
    virtual ~lsScene();
	
    // from lsNode
    void Move( lsScene* scn, lsRender* render, double delta );
    void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
    void InitDrawlist();
	
    // Get the base Scene, every node has this func.
    lsScene *Get_Scene();
    lsEventManager *Get_EventManager();
	
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

    // New: Skybox object
    lsObject* getSkyObject() const { return sky_object; }
    void setSkyObject(lsObject* val) { sky_object = val; }
    
    // Audio Listener
    lsListener* getListener() const { return m_listener; }
	
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
	
    // Skybox object
    lsObject* sky_object;
    // Audio Listener
    lsListener* m_listener;
	
    // Threads
    mutex m_mutex;
};
