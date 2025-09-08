#ifndef vortxGE_lsScene_h
#define vortxGE_lsScene_h

#include <thread>
#include <mutex>

#include "lsNode.h"

class lsBulletWorld;
class lsEvent;
class lsObject;

using namespace std;

/*!
 * \brief The lsScene class must be used for any Scene.
 *
 * The base node for any scene in vortxGE.\n
 * \n
 * Holds an Event Manager that controls all nodes in this scene. e.g. To delete a scene object just create an Event and the Scene node will take care of it safely.\n
 * \n
 * maxProxies - Make sure you set this high enough to cover all your ridged bodies (bullet physics) or bullet will crash.\n
 *
 */
class lsScene : public lsNode
{
public:
    lsScene( float world_size, int maxProxies );
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

private:
    lsObject* draw_list;

    // Events
    lsEventManager *root_eventmanager;
    // physics
    lsBulletWorld   *bulletworld;
    // Found node
    lsNode* found_node;

    // Threads
    mutex m_mutex;

};


#endif
