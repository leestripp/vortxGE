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
#include <assets/lsAsset_Texture.h> // Include for lsAsset_Texture
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
#include <datablocks/lsBatch_Mesh.h> // Include for lsBatch_Mesh to access tex_albedo
#include <datablocks/lsMesh.h>
#include <datablocks/lsFBO_ColourDepth_8.h>
#include <datablocks/lsFBO_Depth.h>
#include <datablocks/lsBulletShape.h>
#include <datablocks/lsTexture.h> // Include for lsTexture

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
#include <scene/lsFirstPersonRig.h> // Changed include to lsFirstPersonRig.h
#include <scene/lsLight.h>
#include <scene/lsLight_Spot.h>
#include <scene/lsLight_Point.h>
#include <scene/lsLight_Direct.h>
#include <scene/lsLightManager.h>
#include <scene/lsSpeaker.h>

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
	m_speaker = nullptr; // Initialize m_speaker
}

DemoScene::~DemoScene()
{
	if(m_vortxge)
	{
		m_vortxge->Cleanup();
		delete m_vortxge;
	}
	// m_speaker is owned by the scene and will be deleted through the scene hierarchy
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
	m_vortxge->getRender()->CheckErrors( "DemoScene::Init() END" );
	
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
	m_quad_viewer = new lsViewer( m_vortxge->getRender(), nullptr );
	if( m_quad_viewer )
	{
		m_quad_viewer->setName( "Quad Viewer" );
		
		// Init
		m_quad_viewer->Init();
		m_vortxge->getControlManager()->Add_Control( m_quad_viewer );
		
		// Create quad scene
		m_quad_scene = new lsScene( m_vortxge->getRender(), 100.0, 1 );
		if( m_quad_scene )
		{
			// Give our scene a name.
			m_quad_scene->setName("Quad Scene");
			
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
			lsPass_DR *pass_dr = new lsPass_DR( m_vortxge->getRender() );
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
	m_viewer = new lsViewer( m_vortxge->getRender(), NULL );
	if( m_viewer )
	{
		m_viewer->setName( "Demo Scene Viewer" );

		// Init
		m_viewer->Init();
		m_vortxge->getControlManager()->Add_Control( m_viewer );

		// Create viewer scene
		// World size 1000 meters
		// maxProxies - Set this high enough for your physics objects or they wont be added to the sim.
		m_viewer_scene = new lsScene( m_vortxge->getRender(), 1000.0, 1000 );
		if( m_viewer_scene )
		{
			m_viewer->getSceneManager()->Add_Scene( m_viewer_scene );
			
			//***********************************************
			// Add Player Rig - instantiate lsFirstPersonRig
			m_playerRig = new lsFirstPersonRig(m_viewer_scene, m_viewer_scene->getBulletworld()->getDynamicsWorld());
			m_playerRig->Set_Pos(0, 5, 8); // Position camera to look at the scene from +Z
			m_viewer_scene->Add_Node(m_playerRig);
			
			lsCamera* playerCam = m_playerRig->getCamera();
			playerCam->Set_Perspective(45.0, 1820.0 / 1080.0, 0.1, 1000.0);
			
			// create FBO pass to render main scene too.
			// This gets applied to the Quad.
			lsPass_FBO *pass_fbo = new lsPass_FBO( m_vortxge->getRender() );
			if( pass_fbo )
			{
				// Create FBO textures at FHD 1920x1080 8bpp
				lsFBO_ColourDepth_8 *lsfbo = new lsFBO_ColourDepth_8( 1920, 1080, m_vortxge->getRender() );
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
			
			// Create a lsLight_Direct to follow the player.
			m_sun_light = new lsLight_Direct(m_viewer_scene);
			if(m_sun_light && playerCam)
			{
				// Set the follow camera.
				m_sun_light->setFollowCamera(playerCam);
				
				// Default settings.
				m_sun_light->Set_Orthographic(-200.0, 200.0, -200.0, 200.0, 1.0, 100.0);
		        m_sun_light->setName("Sun Light");
				m_sun_light->Set_Pos(0.0, 50.0, 30.0);
		        m_sun_light->Set_Rot(-60.0, 0.0, 0.0);
		        // light colour
		        m_sun_light->diffuse = lsVector4(1.0, 0.9, 0.85, 1.0);
		        
				// Add to scene.
				m_viewer_scene->Add_Node(m_sun_light);
		        m_viewer_scene->getLightManager()->Add_Light(m_sun_light);
			}
		}// if viewer_scene
	}// if viewer
}


// ************************************
//  Test Scene
// ************************************

void DemoScene::setupTestScene()
{
	// Skybox Setup
	// Load custom shaders not part of vortxGE's default set
	lsShader* skyboxShader = m_vortxge->getShaderManager()->Find_Shader("skybox");
	if(! skyboxShader)
	{
		skyboxShader = new lsShader(m_vortxge->getRender());
		if(skyboxShader)
		{
			skyboxShader->Load_Shader("skybox");
			// Set shader type. LSST_SKYBOX
			skyboxShader->setKind(ShaderKind::Skybox);
			
			m_vortxge->getShaderManager()->Add_Shader(skyboxShader);
		} else
		{
			cout << "ERROR: DemoScene::Init() : Can't create lsShader instance for skybox." << endl;
		}
	}
	
	// Load the sphere mesh (e.g., from a Blender export named "SkySphere")
	lsAsset_Mesh* sphereMeshAsset = m_viewer_scene->getAssetManager()->CreateAsset_Mesh("data/meshes/SkySphere.glb");
	sphereMeshAsset->AllocAsset();
	
	// Create the skybox object
	lsObject* skyObject = new lsObject(m_viewer_scene);
	if (skyObject)
	{
		skyObject->setName("SkyboxObject");
		skyObject->Set_Asset(sphereMeshAsset); 
		skyObject->Set_Scale(1.0f, 1.0f, 1.0f);		// Already scaled to 1000.0
		skyObject->Set_Pos(0, 0, 0);
		skyObject->Set_Rot(0, 0, 0);
		
		// Assign the skybox shader
		lsShader* skyboxShader = m_vortxge->getShaderManager()->Find_Shader("skybox");
		if (skyboxShader)
		{
			static_cast<lsBatch_Mesh*>(skyObject->Get_Mesh()->getBatch_list())->setShader(skyboxShader);
		} else
		{
			std::cerr << "WARNING: Skybox shader not found! Using default shader." << std::endl;
		}
		// Set as the special sky_object, managed by lsScene.
		m_viewer_scene->setSkyObject(skyObject);
	} else
	{
		std::cerr << "ERROR: Failed to create SkyboxObject." << std::endl;
	}
	
	// Create an infinite ground plane for physics
	lsObject* ground = new lsObject(m_viewer_scene);
	ground->setName("GroundPlane");
	// This object is not rendered, so no mesh asset is needed.
	
	// The plane is defined by its normal vector and a constant.
	// For a ground plane at y=0, the normal is (0, 1, 0).
	lsAsset_BulletShape_Plane* planeShapeAsset = m_viewer_scene->getAssetManager()->CreateAsset_BulletShape_Plane(0, 1, 0);
	planeShapeAsset->AllocAsset();
	lsBulletShape* bulletShape = static_cast<lsBulletShape*>(planeShapeAsset->getData());

	lsBulletObject* bulletObject = new lsBulletObject(ground, bulletShape->getShape(), m_viewer_scene->getBulletworld()->getMax_proxies());
	bulletObject->Init(0.0f); // Mass 0 for static object
	ground->setBulletObject(bulletObject);
	m_viewer_scene->Add_Node(ground);
	
	// Load the demoscene.vgs file
	if (m_viewer)
	{
		m_viewer->getSceneManager()->LoadSceneFromFile("data/demoscene.vgs", m_viewer_scene);
		
		// Manually trigger a single Move pass to update ModelMatrix for all nodes
		// This is necessary so that nodes have their correct world positions for light linking.
		m_viewer_scene->Move(m_viewer_scene, m_vortxge->getRender(), 0.0); // Update scene's own matrix
		m_viewer_scene->MoveThread(m_viewer_scene, m_vortxge->getRender(), 0.0); // Update all children recursively
		
		// After loading and updating transforms, link lights to objects
		m_viewer->getSceneManager()->LinkLightsInScene(m_viewer_scene);
	}
	
	// Add an audio speaker
	m_speaker = new lsSpeaker(m_viewer_scene);
	if (m_speaker)
	{
		m_speaker->setName("SceneSpeaker");
		m_speaker->Set_Pos(0, 5, 0); // Position the speaker in the center of the world
		m_speaker->setGain(0.5f);    // Half volume
		m_speaker->setLooping(true); // Loop the sound
		
		// Load a WAV file from data/audio/sfx
		if (m_speaker->LoadWav("data/audio/sfx/atmos_loop.wav"))
		{
			m_speaker->Play(); // Start playing the sound
		} else
		{
			std::cerr << "WARNING: Could not load or play audio for SceneSpeaker. Make sure data/audio/sfx/atmos_loop.wav exists." << std::endl;
		}
		
		m_viewer_scene->Add_Node(m_speaker); // Add the speaker to the scene
	} else
	{
		std::cerr << "ERROR: Failed to create SceneSpeaker." << std::endl;
	}
	
	// Setup shadowmaps.
	setupShadows();
	
	// Create object emitter.
	setupObjectEmitter();
}

void DemoScene::setupObjectEmitter()
{
	if (!m_viewer_scene) return;
	
	cout << "DemoScene::setupObjectEmitter() called" << endl;
	
	// Create the mesh asset for emitted objects
	lsAsset_Mesh* emitMesh = m_viewer_scene->getAssetManager()->CreateAsset_Mesh("data/meshes/Wood_Cube.glb");
	if (!emitMesh)
	{
		cout << "ERROR: Failed to create mesh asset for emitter" << endl;
		return;
	}
	emitMesh->AllocAsset();
	
	// Create the bullet shape asset (box with size 1.0, 1.0, 1.0)
	lsAsset_BulletShape_Box* emitBulletShape = m_viewer_scene->getAssetManager()->CreateAsset_BulletShape_Box(1.0f, 1.0f, 1.0f);
	if (!emitBulletShape)
	{
		cout << "ERROR: Failed to create bullet box shape for emitter" << endl;
		return;
	}
	emitBulletShape->AllocAsset();
	
	// Create the emitter object
	lsObjectEmitter* emitter = new lsObjectEmitter(emitMesh, emitBulletShape, m_viewer_scene);
	if (!emitter)
	{
		cout << "ERROR: Failed to create object emitter" << endl;
		return;
	}
	
	// Set emitter properties
	emitter->setName("WoodCubeEmitter");
	emitter->Set_Pos(0, 20, -10); // Position above the ground
	emitter->setEmittime(500.0); // Emit every 500ms
	emitter->setEmit_life(20.0 * 1000.0); // Life of 20 seconds
	
	// Set randomization properties for more interesting emission
	emitter->setRand_area(lsVector3(2.0f, 0.0f, 2.0f)); // Random area around emitter
	emitter->setRand_velocity(lsVector3(2.0f, 5.0f, 2.0f)); // Random velocity
	emitter->setRand_rotation(true); // Random rotation
	
	// Add lights
	emitter->add_Emit_Light(m_sun_light);
	
	// Add the emitter to the scene
	m_viewer_scene->Add_Node(emitter);
	
	cout << "DemoScene::setupObjectEmitter() completed" << endl;
}

void DemoScene::setupShadows()
{
	if (!m_viewer || !m_viewer_scene) return;
	
	cout << "DemoScene::setupShadows() called" << endl;
	
	// Get all lights from the scene's central light manager
	const auto& lights = m_viewer_scene->getLightManager()->getLights();
	std::vector<lsPass*> shadow_passes;
	
	for (lsLight* light : lights)
	{
		// Add shadow maps to spot and direct lights
		lsPass_ShadowMap* shadowPass = nullptr;
		if( light->getLight_type() == LSLT_DIRECT)
		{
			// Create the shadow map pass (which creates its own FBO)
			shadowPass = new lsPass_ShadowMap(0, 0, 4096, 4096, m_vortxge->getRender());
		}
		if (light->getLight_type() == LSLT_SPOT)
		{
			// Create the shadow map pass (which creates its own FBO)
			shadowPass = new lsPass_ShadowMap(0, 0, 2048, 2048, m_vortxge->getRender());
		}
		if(! shadowPass)
		{
			cerr << "ERROR: Failed to create shadow pass." << endl;
			continue;
		}
		// debug
		cout << "  Found Shadow Map Light '" << light->getName() << "', adding shadow map." << endl;
		
		// Configure the pass
		shadowPass->setScene(m_viewer_scene);
		shadowPass->setCamera(light); // The light acts as the camera for rendering the shadow map
		shadowPass->setCull_camera(light);
		shadowPass->setLight(light);
		
		// Link the shadow map texture back to the light
		if(shadowPass->getFBO())
		{
			light->setTex_shadowmap(shadowPass->getFBO()->getDepthTexture());
			light->setHas_shadowmap(true);
		}
		
		// Add the pass to the viewer's pass manager.
		// lsPassManager adds to the front, so this will execute before the main render pass.
		m_viewer->getPassManager()->Add_Pass(shadowPass);
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
	if (m_playerRig)
	{
		m_playerRig->HandleKeyboardInput(f, b, l, r, crouch, jump, sprint);
	}
}

void DemoScene::rotatePlayerView( double delta_x, double delta_y)
{
	if (m_playerRig) {
		m_playerRig->HandleMouseLook(1.0f / 60.0f, static_cast<float>(delta_x), static_cast<float>(delta_y)); // Assuming 60 FPS
	}
}

void DemoScene::handleMouseButtons(bool left, bool right)
{
	if (m_playerRig)
	{
		m_playerRig->HandleMouseButtons(left, right);
	}
}

