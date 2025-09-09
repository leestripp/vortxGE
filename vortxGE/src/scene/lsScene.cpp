#include "../vortx_debug.h"

// OpenGL
#include "../opengl/lsOpenGL.h"

// datablocks
#include "../datablocks/lsStats.h"

// Events
#include "../events/lsEventManager.h"
#include "../events/lsEvent.h"
#include "../events/lsEvent_HitPoint.h"
#include "../events/lsEvent_DelNode.h"

// Physics
#include "../physics/lsBulletWorld.h"

// Assets
#include "../assets/lsAssetManager.h"

#include "lsObject.h"
#include "lsScene.h"


lsScene::lsScene( lsOpenGL* ogl, float world_size, int maxProxies ) : lsNode( NULL )
{
    setNode_type( LSNT_SCENE );
    setName( "Scene" );

    // Assets
    m_assetManager = new lsAssetManager(ogl);
    if (!m_assetManager)
    {
        // ERROR
        cout << "ERROR: lsScene::lsScene - Could not instance lsAssetManager" << endl;
    }

    // Physics
    bulletworld = new lsBulletWorld( this );
    if(! bulletworld )
    {
        // ERROR
        cout << "ERROR: lsScene::lsScene - Could not instance lsBulletWorld" << endl;
    } else
    {
        // Set world size
        bulletworld->setWorld_size( world_size );
        bulletworld->setMax_proxies( maxProxies );

        bulletworld->Init();
    }

    // Events
    root_eventmanager = new lsEventManager();
    if(! root_eventmanager )
    {
        // ERROR
        cout << "ERROR : lsScene::lsScene - Could not instance lsEventManager" << endl;
    }

    draw_list = NULL;
}

lsScene::~lsScene()
{
    // Scene Manager cleans up nodes

    // Physics
    if( bulletworld ) delete bulletworld;

    // Assets
    if (m_assetManager) delete m_assetManager;

#ifdef VORTX_DEBUG_NODES
    cout << "lsScene::~lsScene() - Name: " << getName() << endl;
#endif
}

/*!
 * Poll scene events, Update physics and Init Identity Matrix
 *
 * \param scn
 * \param opengl
 * \param delta
 */
void lsScene::Move( __attribute__((unused)) lsScene* scn, lsOpenGL* opengl, double delta )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsScene::Move : Process events, Physics Step, Init ModelMatrix" << endl;
#endif

    // Base Matrix
    ModelMatrix.LoadIdentity();
	
    // Update physics engine
    if( opengl->getStats() ) opengl->getStats()->Physics_Start();
    if( bulletworld ) bulletworld->BulletStep( 10, delta );
    if( opengl->getStats() ) opengl->getStats()->Physics_Stop();

#ifdef VORTX_DEBUG_NODES
    ModelMatrix.debug( "lsScene ModelMatrix: " + getName() );
#endif
}

/**
 InitDrawlist ready for drawing\n
 Called from Passes
 */
void lsScene::InitDrawlist()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsScene::InitDrawlist : Draw List set to NULL" << endl;
#endif
    draw_list = NULL;
}

/*!
 * \brief lsScene::Draw
 *
 * Calls Draw() on the Draw list. Also starts and stops stats for Draw.
 * \n
 *
 * \param cam
 * \param pass
 * \n
 *
 * \param object
 */
void lsScene::Add_DrawObject( lsObject *object )
{
    object->setPrev_drawobject( NULL );

    m_mutex.lock();
    object->setNext_drawobject( draw_list );
    if( draw_list ) draw_list->setPrev_drawobject( object );
    draw_list = object;
    m_mutex.unlock();
}

/*! Add event
 Thread safe
 */
void lsScene::Add_Event( lsEvent *event )
{
    m_mutex.lock();
    Get_EventManager()->Add_Event( event );
    m_mutex.unlock();
}

/*!
 * \brief Get the next event
 *
 * Calls the Event Managers function PollEvent()
 * Event manager deletes the event for us.
 *
 * \return
 */
lsEvent* lsScene::Poll_Event()
{
    lsEvent* event = NULL;
    lsEvent* delnode = NULL;

    if( Get_EventManager() )
    {
    	event = Get_EventManager()->Poll_Event();
        while(  event )
        {
            switch( event->getEvent_type() )
            {
            case LSET_DEL_NODE:
                delnode = (lsEvent_DelNode*)event;
                Del_Node( delnode->getNode() );
                break;

                // User events
            case LSET_HIT_POINT:
                return event;
                break;

            default:
                cout << "ERROR: Unknown scene event" << endl;
                break;
            }
            // get next event
            event = Get_EventManager()->Poll_Event();
        } // while loop
    }

    return event;
}

/** Delete node
 Delets a single node
 */
void lsScene::Del_Node( lsNode* node )
{
#if defined(VORTX_DEBUG_NODES)
    cout << "lsScene::Del_Node()" << endl;
#endif

    // Do recursive delete of attached nodes
    if( node->getNode_list() )
    {
        node->getNode_list()->Del_AllNodes();
        // delete base node
        delete node->getNode_list();
        node->setNode_list( NULL );
    }

    // Remove node from its parent
    lsNode* prev;
    lsNode* next;
    lsNode* par;

    par = node->getParent();
    prev = node->getPrev_node();
    next = node->getNext_node();
    // close gap
    if( prev )
    {
        prev->setNext_node( next );
        if( next ) next->setPrev_node( prev );

    } else
    {
        par->setNode_list( next );
        if( next ) next->setPrev_node( 0 );
    }

    delete node;

}

// ####################
// Save data

const string& lsScene::toString()
{
    string tmp;

    save_str = "add_scene ";
    if( getName() != "" )
    {
    	save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "Scene";
    }

    return save_str;
}

