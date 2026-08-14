#include <algorithm> 

// Debug
#include "../vortx_debug.h"

// Render
#include "../render/lsRender.h"

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

// Audio
#include "lsListener.h" // Include lsListener header

#include "lsObject.h"
#include "lsScene.h"
#include "lsCamera.h"


lsScene::lsScene( lsRender* m_render, float world_size, int maxProxies ) : lsNode( NULL )
{
	setNode_type( LSNT_SCENE );
	setName( "Scene" );

	// Assets
	m_assetManager = new lsAssetManager(m_render);
	if (!m_assetManager)
	{
		// ERROR
		cout << "ERROR: lsScene::lsScene - Could not instance lsAssetManager" << endl;
	}
	
	// Lights : used for fast lookup of lights.
	m_lightManager = new lsLightManager();
	if(! m_lightManager)
	{
		cout << "ERROR: lsScene::lsScene - Could not instance lsLightManager" << endl;
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

	// Audio Listener
	m_listener = new lsListener(this); // Create a default listener for the scene
	if (!m_listener)
	{
		cerr << "ERROR: lsScene::lsScene - Could not instance lsListener" << endl;
	} else
	{
		Add_Node(m_listener); // Add listener as a child of the scene
	}
	
	draw_list = nullptr;
	sky_object = nullptr;
}

lsScene::~lsScene()
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsScene::~lsScene() START - Name: " << getName() << endl;
#endif
	
	// Destroy all child nodes FIRST. Physics-backed nodes (lsBulletObject
	// dtors) call dynamicsWorld->removeRigidBody(), so they must die while
	// the Bullet world is still alive. Without this, base-class ~lsNode
	// destroys m_children AFTER this body has already deleted bulletworld
	// (use-after-free on exit).
	getChildren().clear();

	// Sky object is not part of m_children
	if(sky_object) delete sky_object;

	// Physics
	if( bulletworld ) delete bulletworld;
	
	// Lights
	if (m_lightManager) delete m_lightManager;
	
	// Assets
	if (m_assetManager) delete m_assetManager;
	
#ifdef VORTX_DEBUG_NODES
	cout << "lsScene::~lsScene() END - Name: " << getName() << endl;
#endif
}

/*!
 * Update physics and Set Identity Matrix
 *
 * \param scn
 * \param render
 * \param delta
 */
void lsScene::Move( __attribute__((unused)) lsScene* scn, lsRender* render, double delta )
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsScene::Move : Process events, Physics Step, Init ModelMatrix" << endl;
#endif

	// Base Matrix
	ModelMatrix.LoadIdentity();
	
	// Update physics engine
	if( render->getStats() ) render->getStats()->Physics_Start();
	if( bulletworld ) bulletworld->BulletStep( 10, delta ); // Fixed: BulletStep
	if( render->getStats() ) render->getStats()->Physics_Stop();

#ifdef VORTX_DEBUG_NODES
	ModelMatrix.debug( "lsScene ModelMatrix: " + getName() );
#endif
}

//******************
// Optimize

void lsScene::Optimize( lsCamera* cam, __attribute__((unused)) int pass_type, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render )
{
	// Update Actice light based on distance from camera.
	// TODO: create a setable distance for the user.
	m_lightManager->setActiveLights(cam, 50.0);

	// NOTE: asset streaming is NOT driven from here. Optimize() runs
	// per-pass_type with whatever cull camera is active (including shadow-pass_type
	// light cameras), which made the streaming manager see conflicting
	// camera positions and flap load/evict. The app drives
	// lsStreamingManager::update() once per frame from the main thread
	// with the viewer camera (see DemoScene::Update).
}

/*!
 * \brief lsScene::Draw
 *
 * Calls Draw() on the Draw list. Also starts and stops stats for Draw.
 * \n
 *
 * \param cam
 * \param pass_type
 * \param scn
 * \param render
 */
void lsScene::Draw( lsCamera* cam, int pass_type, __attribute__((unused)) lsScene* scn, lsRender* render )
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsScene::Draw" << endl;
#endif
	// Draw stats
	if( render->getStats() ) render->getStats()->Draw_Start();
	
	// Draw Skybox first
	if(sky_object)
	{
		// Set depth function to draw the skybox (which is fixed at max depth 1.0)
		//    We use LS_LEQUAL to allow it to pass_type even when its depth equals the far plane's depth.
		render->DepthFunc( LS_LEQUAL );

		// Disable depth writing (We don't want the skybox to obscure other objects' depth)
		render->DepthMask( LS_FALSE );

		// Disable culling for skybox geometry (so we see the inside of the cube/sphere)
		render->Disable( LS_CULL_FACE );

		sky_object->Draw( cam, pass_type, this, render );

		// 4. Restore state after drawing skybox
		render->Enable( LS_CULL_FACE );
		render->DepthMask( LS_TRUE );

		// Restore the default depth function for the rest of the scene
		render->DepthFunc( LS_LESS );
	}
	
	// Draw remaining scene objects
	if( draw_list ) draw_list->Draw( cam, pass_type, this, render );
	
	if( render->getStats() ) render->getStats()->Draw_Stop();
}

/*!
 * \brief lsScene::Get_Scene
 *
 * Only use when building you scene. Could be slow on a huge heirarchy.\n
 * \n
 *
 * \return - Returns the base scene of a lsNode structure.
 */
lsScene* lsScene::Get_Scene()
{
	return this;
}

/*!
 * \brief lsScene::Get_EventManager
 *
 * Only use when building you scene. Could be slow on a huge heirarchy.\n
 * \n
 *
 * \return - Returns the base scene Event Manager.
 */
lsEventManager* lsScene::Get_EventManager()
{
	return root_eventmanager;
}

/*!
 * \brief lsScene::ClearScene
 *
 * Deletes everything in this scene but leave the scene node.\n
 * \n
 *
 */
void lsScene::ClearScene()
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsScene::ClearScene() - Scene Name: " << getName() << "  *******************" << endl;
#endif
	
	// nodes are unique so this will clear the entire scene.
	Del_Children();
	// But not the sky_object its not part of the scene directly.
	if(sky_object) delete sky_object;
	sky_object = nullptr;
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
 * \param pass_type
 * \n
 *
 * \param object
 */
void lsScene::Add_DrawObject( lsObject *object )
{
	object->setPrev_drawobject( NULL );
	
	object->setNext_drawobject( draw_list );
	if( draw_list ) draw_list->setPrev_drawobject( object );
	draw_list = object;
}

/*! Add event
 Thread safe
 */
void lsScene::Add_Event( lsEvent *event )
{
	Get_EventManager()->Add_Event( event );
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
				// Process the deletion through the new mechanism
				delnode->ProcessEvent();
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
