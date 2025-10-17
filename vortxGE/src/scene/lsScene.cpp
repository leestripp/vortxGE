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
		std::cerr << "ERROR: lsScene::lsScene - Could not instance lsListener" << std::endl;
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
	
	// Scene Manager cleans up nodes
	// But not the sky_object
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


/*!
 * \brief lsScene::Draw
 *
 * Calls Draw() on the Draw list. Also starts and stops stats for Draw.
 * \n
 *
 * \param cam
 * \param pass
 * \param scn
 * \param render
 */
void lsScene::Draw( lsCamera* cam, lsPass* pass, __attribute__((unused)) lsScene* scn, lsRender* render )
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
		//    We use GL_LEQUAL to allow it to pass even when its depth equals the far plane's depth.
		glDepthFunc(GL_LEQUAL); 
		
		// Disable depth writing (We don't want the skybox to obscure other objects' depth)
		glDepthMask(GL_FALSE); 
		
		// Disable culling for skybox geometry (so we see the inside of the cube/sphere)
		glDisable(GL_CULL_FACE); 
		
		sky_object->Draw(cam, pass, this, render);

		// 4. Restore state after drawing skybox
		glEnable(GL_CULL_FACE); 
		glDepthMask(GL_TRUE); 
		
		// Restore the default depth function for the rest of the scene
		glDepthFunc(GL_LESS); 
	}
	
	// Draw remaining scene objects
	if( draw_list ) draw_list->Draw( cam, pass, this, render );
	
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
 * \param pass
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
