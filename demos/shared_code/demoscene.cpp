#include <iostream>

// GLEW
#include <GL/glew.h>
// m_vortxge
#include <vortxGE.h>

// Events
#include <events/lsEventManager.h>
#include <events/lsEvent_HitPoint.h>

// Math
#include <math/lsVector4.h>
#include <math/lsVector3.h>

// Shaders
#include <shader/lsShaderManager.h>
#include <shader/lsShader.h>

// Assets
#include <assets/lsAssetManager.h>
#include <assets/lsAsset.h>
#include <assets/lsAsset_Mesh.h>
#include <assets/lsAsset_Quad.h>
#include <assets/lsAsset_MeshGuide_Empty.h>
#include <assets/lsAsset_MeshGuide_Grid.h>
// bullet
#include <assets/lsAsset_BulletShape_Box.h>
#include <assets/lsAsset_BulletShape_Plane.h>
#include <assets/lsAsset_BulletShape_ConvexHull.h>
#include <assets/lsAsset_BulletShape_TriMesh.h>

// Physics
#include <physics/lsBulletObject.h>
#include <physics/lsBulletWorld.h>

// datablocks
#include <datablocks/lsBatch.h>
#include <datablocks/lsBatch_Lines.h>
#include <datablocks/lsBatch_Quad.h>
#include <datablocks/lsMesh.h>
#include <datablocks/lsFBO_ColourDepth_8.h>
#include <datablocks/lsFBO_Depth.h>
#include <datablocks/lsBulletShape.h>

// Controls
#include <controls/lsControlManager.h>
#include <controls/lsControl.h>
#include <controls/lsComposite.h>
#include <controls/lsViewer.h>

// Passes
#include <passes/lsPassManager.h>
#include <passes/lsPass.h>
#include <passes/lsPass_DR.h>
#include <passes/lsPass_FBO.h>
#include <passes/lsPass_Occlusion.h>
#include <passes/lsPass_ShadowMap.h>

// Scene
#include <scene/lsSceneManager.h>
#include <scene/lsNode.h>
#include <scene/lsScene.h>
#include <scene/lsObject.h>
#include <scene/lsObjectEmitter.h>
#include <scene/lsCamera.h>
#include <scene/lsLight_Spot.h>
#include <scene/lsNode_FirstPersonRig.h> // Include the new rig

// Light manager
#include <lights/lsLightManager.h>

// Animation
#include <animation/lsAnimationManager.h>

#include "demoscene.h"


// Callback for OpenGL errors.
void GLAPIENTRY MessageCallback( GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	__attribute__((unused)) GLsizei length,
	const GLchar* message,
	__attribute__((unused)) const void* userParam )
{
	// Don't print notification messages from the driver
	if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		return;
	}
	
	cerr << "ERROR: " << ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "unknown" );
	cerr << ", message: " << message;
	cerr << ", source: " << source;
	cerr << ", id: " << id;
	cerr << ", severity: " << severity;
	cerr << ", id: " << id;
	cerr << endl;
}

DemoScene::DemoScene()
{
	m_quad_viewer = nullptr;
	m_quad_scene = nullptr;
	
	m_viewer = nullptr;
	m_viewer_scene = nullptr;
	
	m_playerRig = nullptr;
	m_spot_light = nullptr;
}

DemoScene::~DemoScene()
{
	if(m_vortxge)
	{
		m_vortxge->Cleanup();
		delete m_vortxge;
	}
}


// ************************************
//  Setup
// ************************************

bool DemoScene::Init()
{
	cout << "DemoScene::Init() called" << endl;

	// create m_vortxge - must have an active GL context
	m_vortxge = new vortxGE();
	if( m_vortxge )
	{
		// start GLEW extension handler
		glewExperimental = GL_TRUE;
		glewInit();

		// Need debug context and extensions loaded first.
		glEnable( GL_DEBUG_OUTPUT );
		glDebugMessageCallback( MessageCallback, 0 );

		m_vortxge->Init();

		// Set some openGL defaults
		glFrontFace( GL_CCW );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );

		setupControls();
		setupTestScene();

	} else
	{
		cout << "ERROR: DemoScene::Init() : Can't create m_vortxge instance." << endl;
		return false;
	}

	// Check GL errors
	m_vortxge->getOpenGL()->CheckGLErrors( "DemoScene::Init() END" );
	
	return true;
}

void DemoScene::Resize( int x, int y, int w, int h )
{
	// cout << "DemoScene::Resize" << endl;
	
	if(! m_vortxge ) return;
	m_vortxge->Resize( x, y, w, h );
}

// ************************************
//  Controls
// ************************************

void DemoScene::setupControls()
{
	cout << "DemoScene::setupControls() called" << endl;

	// pass_fbo needs to pass its texture to this.
	lsAsset_Quad *asset_quad = NULL;
	
	// Add quad viewer control
	m_quad_viewer = new lsViewer( m_vortxge->getOpenGL(), NULL );
	if( m_quad_viewer )
	{
		m_quad_viewer->setName( "Quad Viewer" );
		
		// Init
		m_quad_viewer->Init();
		m_vortxge->getControlManager()->Add_Control( m_quad_viewer );
		
		// Create quad scene
		m_quad_scene = new lsScene( m_vortxge->getOpenGL(), 100.0, 1 );
		if( m_quad_scene )
		{
			m_quad_viewer->getSceneManager()->Add_Scene( m_quad_scene );
			
			// Add Camera
			lsCamera *quad_camera = new lsCamera( m_quad_scene );
			if( quad_camera )
			{
				// Orthographic camera defaults
				quad_camera->Set_Orthographic( 1.0 );

				m_quad_scene->Add_Node( quad_camera );
			}

			// Create Quad
			lsObject *quad = new lsObject( m_quad_scene );
			if( quad )
			{
				quad->setName( "Quad" );
				quad->Set_Rot( 0, 180.0, 0 );
				
				// Use quad asset
				asset_quad = m_quad_scene->getAssetManager()->CreateAsset_Quad();
				asset_quad->AllocAsset();
				quad->Set_Asset( asset_quad );

				m_quad_scene->Add_Node( quad );
			}

			// Create Direct render Pass
			lsPass_DR *pass_dr = new lsPass_DR( m_vortxge->getOpenGL() );
			if( pass_dr )
			{
				// If the user see's this colour, Quad could be backwards, not rendering.
				pass_dr->setBackground_colour( lsVector4( 0.5, 0, 0, 1.0 ) );
				
				m_quad_viewer->getPassManager()->Add_Pass( pass_dr );
				pass_dr->setScene( m_quad_scene );
				pass_dr->setCamera( quad_camera );
				pass_dr->setCull_camera( quad_camera );
			}
		}// if( m_quad_scene )
	}// if m_quad_viewer
	
	
	// Add demoscene viewer control
	m_viewer = new lsViewer( m_vortxge->getOpenGL(), NULL );
	if( m_viewer )
	{
		m_viewer->setName( "Demo Scene Viewer" );

		// Init
		m_viewer->Init();
		m_vortxge->getControlManager()->Add_Control( m_viewer );

		// Create viewer scene
		// World size 1000 meters
		// maxProxies - Set this high enough for your physics objects or they wont be added to the sim.
		m_viewer_scene = new lsScene( m_vortxge->getOpenGL(), 1000.0, 1000 );
		if( m_viewer_scene )
		{
			m_viewer->getSceneManager()->Add_Scene( m_viewer_scene );

			// Add Player Rig
			m_playerRig = new lsNode_FirstPersonRig(m_viewer_scene);
			m_playerRig->Set_Pos(0, 5, 8); // Position camera to look at the scene from +Z
			m_viewer_scene->Add_Node(m_playerRig);
			
			lsCamera* playerCam = m_playerRig->getCamera();
			playerCam->Set_Perspective(45.0, 1820.0 / 1080.0, 0.1, 1000.0);
			
			// create FBO pass to render main scene too.
			// This gets applied to the Quad.
			lsPass_FBO *pass_fbo = new lsPass_FBO( m_vortxge->getOpenGL() );
			if( pass_fbo )
			{
				// Create FBO textures at FHD 1920x1080 8bpp
				lsFBO_ColourDepth_8 *lsfbo = new lsFBO_ColourDepth_8( 1920, 1080, m_vortxge->getOpenGL() );
				if( lsfbo )
				{
					pass_fbo->setFBO( lsfbo );

					// Connect the Quad
					asset_quad->Set_texture( lsfbo->getTexture() );
				}
				// Black background for our main scene.
				pass_fbo->setBackground_colour( lsVector4( 0, 0, 0, 1.0 ) );

				m_viewer->getPassManager()->Add_Pass( pass_fbo );
				pass_fbo->setScene( m_viewer_scene );
				pass_fbo->setCamera( playerCam );
				pass_fbo->setCull_camera( playerCam );
			}// if pass_fbo
		}// if viewer_scene
	}// if viewer
}


// ************************************
//  Test Scene
// ************************************

void DemoScene::setupTestScene()
{
	cout << "DemoScene::setupTestScene() called" << endl;
	
	lsObject *debug_empty;
	
	// Create a spot light for shadows
	m_spot_light = new lsLight_Spot(m_viewer_scene);
	if(m_spot_light)
	{
		// void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
		m_spot_light->Set_Perspective(45.0, 1.0, 0.1, 100.0);
		
		m_spot_light->setName("Shadow Spot Light");
		m_spot_light->Set_Pos(0, 15, 10); // Position light high and behind camera to cast shadows forward
		m_spot_light->Set_Rot(-45, 0, 0); // Pointing downwards at an angle
		// Watts.
		m_spot_light->setIntensity(800.0f);
		
		m_viewer_scene->Add_Node(m_spot_light);
		
		// Add a debug empty to visualize the light's position and orientation
		debug_empty = new lsObject(m_spot_light);
		if(debug_empty)
		{
			lsAsset_MeshGuide_Empty *asset_empty;
			asset_empty = m_viewer_scene->getAssetManager()->CreateAsset_MeshGuide_Empty();
			asset_empty->AllocAsset();
			debug_empty->Set_Asset(asset_empty);
			m_spot_light->Add_Node(debug_empty);
		}
	}
	
	// The main render pass needs to be added before the shadow pass, because lsPassManager
	// adds new passes to the front of the list. This ensures the shadow pass runs first.
	// The main FBO pass is already added in setupControls(), so we can add the shadow pass here.
	lsPass_ShadowMap *pass_shadow = new lsPass_ShadowMap(0, 0, 1024, 1024, m_vortxge->getOpenGL());
	if(pass_shadow)
	{
		pass_shadow->setScene(m_viewer_scene);
		pass_shadow->setCamera(m_spot_light); // The light acts as the camera for the depth pass
		pass_shadow->setCull_camera(m_spot_light); // Culling is also from the light's perspective
		pass_shadow->setLight(m_spot_light);
		m_viewer->getPassManager()->Add_Pass(pass_shadow);
		
		// Link the generated depth texture to the light so shaders can access it
		if(m_spot_light && pass_shadow->getFBO())
		{
			m_spot_light->setHas_shadowmap(true);
			m_spot_light->setTex_shadowmap(pass_shadow->getFBO()->getDepthTexture());
		} else
		{
				cerr << "ERROR: No spot light or depth FBO found for Shadow pass." << endl;
		}
	}
	
	// Add some test Objects
	lsObject *object;

	// Floor
	object = new lsObject( m_viewer_scene );
	if( object )
	{
		object->setName( "Floor" );
		object->setCasts_shadows(false); // The floor doesn't need to cast shadows on itself
		object->setReceives_shadows(true);
		
		// Load Mesh asset
		lsAsset_Mesh *asset_mesh;
		asset_mesh = m_viewer_scene->getAssetManager()->CreateAsset_Mesh( "data/meshes/gltf/floor.glb" );
		asset_mesh->AllocAsset();
		object->Set_Asset( asset_mesh );
		
		// Assign the shadow-casting light to this object
		object->getLightManager()->Add_Light( m_spot_light );
		m_viewer_scene->Add_Node( object );
	}

	// Monkey
	object = new lsObject( m_viewer_scene );
	if( object )
	{
		object->setName( "Monkey" );
		object->Set_Pos(-1.5, 5.0, 0);
		object->setCasts_shadows(true);
		object->setReceives_shadows(true);

		// Load Mesh asset
		lsAsset_Mesh *asset_mesh;
		asset_mesh = m_viewer_scene->getAssetManager()->CreateAsset_Mesh( "data/meshes/gltf/monkey.glb" );
		asset_mesh->AllocAsset();
		object->Set_Asset( asset_mesh );

		// Add Convex Hull physics
		lsAsset_BulletShape_ConvexHull* asset_bsh = m_viewer_scene->getAssetManager()->CreateAsset_BulletShape_ConvexHull(asset_mesh);
		if (asset_bsh)
		{
			asset_bsh->AllocAsset();
			lsBulletShape* bulletshape = static_cast<lsBulletShape*>(asset_bsh->getData());
			lsBulletObject* bo = new lsBulletObject(object, bulletshape->getShape(), m_viewer_scene->getBulletworld()->getMax_proxies());
			bo->Init(10.0f); // 10kg mass, dynamic
		}

		// Assign the shadow-casting light to this object
		object->getLightManager()->Add_Light( m_spot_light );
		m_viewer_scene->Add_Node( object );
	}
	
	// Cube
	object = new lsObject( m_viewer_scene );
	if( object )
	{
		object->setName( "GLTF_Cube" );
		object->Set_Pos(-1.5, 1.0, 0);
		// Shadows
		object->setCasts_shadows(true);
		object->setReceives_shadows(true);
		
		// Load Mesh asset
		lsAsset_Mesh *asset_mesh;
		asset_mesh = m_viewer_scene->getAssetManager()->CreateAsset_Mesh( "data/meshes/gltf/cube.glb" );
		asset_mesh->AllocAsset();
		object->Set_Asset( asset_mesh );

		// Add TriMesh physics
		lsAsset_BulletShape_TriMesh* asset_bst = m_viewer_scene->getAssetManager()->CreateAsset_BulletShape_TriMesh(asset_mesh);
		if (asset_bst)
		{
			asset_bst->AllocAsset();
			lsBulletShape* bulletshape = static_cast<lsBulletShape*>(asset_bst->getData());
			lsBulletObject* bo = new lsBulletObject(object, bulletshape->getShape(), m_viewer_scene->getBulletworld()->getMax_proxies());
			bo->Init(0.0f); // 0 mass, static
		}

		// Assign the shadow-casting light to this object
		object->getLightManager()->Add_Light( m_spot_light );
		m_viewer_scene->Add_Node( object );
	}
	
	// Object emitter
	setupObjectEmitter();
}

void DemoScene::setupObjectEmitter()
{
	lsObject *object;
	
	// Floor Collision
	object = new lsObject( m_viewer_scene );
	if( object )
	{
		object->setName( "Floor Collsion" );

		// Create bullet shape
		lsAsset_BulletShape_Plane *asset_bsp = m_viewer_scene->getAssetManager()->CreateAsset_BulletShape_Plane( 0, 1, 0 );
		if( asset_bsp )
		{
			asset_bsp->AllocAsset();
			
			lsBulletShape *bulletshape;
			bulletshape = (lsBulletShape *)asset_bsp->getData();
			
			// create a bullet object
			lsBulletObject *bo = new lsBulletObject( object, bulletshape->getShape(), m_viewer_scene->getBulletworld()->getMax_proxies() );
			
			// Connects it all up and checks btcollisonWorld before adding ridged body.
			bo->Init( 0.0 ); // 0 = static
		}
		m_viewer_scene->Add_Node( object );
	}

	// Load Mesh asset
	lsAsset_Mesh *asset_mesh;
	asset_mesh = m_viewer_scene->getAssetManager()->CreateAsset_Mesh( "data/meshes/gltf/cube.glb" );
	if( asset_mesh )
	{
		asset_mesh->AllocAsset();
	} else return;
	
	// Create bullet shape
	lsAsset_BulletShape_Box *asset_bs = m_viewer_scene->getAssetManager()->CreateAsset_BulletShape_Box( 1.0, 1.0, 1.0 );
	if( asset_bs )
	{
		asset_bs->AllocAsset();
	} else return;
	
	// Add emitter to scene
	lsObjectEmitter *oe = new lsObjectEmitter( asset_mesh, asset_bs, m_viewer_scene );
	if( oe )
	{
		oe->setName( "Object Emitter" );
		oe->Set_Pos( 0, 20, -15 ); // Position emitter to drop objects in front of the camera
		oe->setMass(50.0); // Set mass of emitted objects.
		oe->setEmit_life( 20 * 1000 ); // in ms
		oe->setEmittime( 500 );	// in ms
		
		oe->setEmit_light( m_spot_light );
		
		m_viewer_scene->Add_Node( oe );
	}
}

void DemoScene::Update( double delta )
{
	lsEvent *e;
	
	// quad scene events.
	while( (e = m_quad_scene->Poll_Event()) )
	{
		// Discard.
	}
	
	// process scene events
	while( (e = m_viewer_scene->Poll_Event()) )
	{
		processEvent( e );
	}
	
	// m_vortxge update
	if( m_vortxge )
	{
		m_vortxge->Update( delta );
	}
}

void DemoScene::Render()
{
	// cout << "DemoScene::Render START" << endl;
	
	// Render
	if( m_vortxge )
	{
		m_vortxge->Render();
	}
	
	// cout << "DemoScene::Render END" << endl;
}


void DemoScene::processEvent( lsEvent* event )
{
	switch( event->getEvent_type() )
	{
	case LSET_HIT_POINT:
		lsEvent_HitPoint *hit = (lsEvent_HitPoint*)event;
		if( hit )
		{
			// TODO: Create a debug empty at the hit point.
		}
		break;
	}
}

void DemoScene::movePlayer(bool f, bool b, bool l, bool r, bool jump, bool sprint, bool crouch)
{
	if (m_playerRig) {
		m_playerRig->handleInput(f, b, l, r, jump, sprint, crouch);
	}
}

void DemoScene::rotatePlayerView( double delta_x, double delta_y)
{
	if (m_playerRig) {
		m_playerRig->handleMouseLook(delta_x, delta_y);
	}
}

