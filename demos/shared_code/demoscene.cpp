/**
 * @file demoscene.cpp
 * @brief Demo scene implementation
 * 
 * This file implements a demo scene that showcases the vortxGE engine's
 * capabilities. It demonstrates:
 * - Engine initialization and setup
 * - Scene creation and object management
 * - Camera and lighting setup
 * - Physics integration
 * - Asset loading (meshes, textures)
 * - Animation system
 * - Post-processing pipeline
 * - User controls and input handling
 * 
 * The demo creates a 3D scene with various objects including:
 * - Ground plane with physics
 * - Various mesh objects (cubes, spheres, etc.)
 * - Directional and point lights
 * - Shadow mapping
 * - Physics-enabled objects
 * 
 * @see demoscene.h
 * @see vortxGE.h
 */

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

// Plan.md Part 3: render pipeline (R2)
#include <pipeline/lsRenderPipeline.h>
#include <pipeline/lsSceneStage.h>
#include <pipeline/lsBlitStage.h>
#include <pipeline/lsShadowStage.h>
#include <pipeline/lsCsmStage.h>
#include <pipeline/lsPrepassStage.h>

// Scene
#include <scene/lsSceneManager.h>
#include <scene/lsNode.h>
#include <scene/lsScene.h>
#include <functional>
#include <scene/lsObject.h>
#include <scene/lsObjectEmitter.h>
#include <scene/lsCamera.h>
#include <scene/lsFirstPersonRig.h> // Changed include to lsFirstPersonRig.h
#include <scene/lsLight.h>
#include <scene/lsLight_Spot.h>
#include <scene/lsLight_Point.h>
#include <scene/lsLight_Direct.h>
#include <scene/lsLightManager.h>
#include <pipeline/lsPointShadowStage.h>
#include <pipeline/lsClusterStage.h>
#include <debug/lsProfiler.h>
#include <filesystem/lsDir.h>
#include <sys/stat.h>
#include <scene/lsSpeaker.h>

// Animation
#include <animation/lsAnimationManager.h>

#include "demoscene.h"


/**
 * @brief OpenGL debug message callback
 * 
 * This callback is called by OpenGL when debug messages are generated.
 * It filters out notification-level messages and prints errors
 * and warnings to stderr.
 * 
 * @param source OpenGL message source
 * @param type Message type (error, warning, etc.)
 * @param id Message ID
 * @param severity Message severity
 * @param length Message length
 * @param message Message string
 * @param userParam User parameter (unused)
 */

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

/**
 * @brief Construct demo scene
 * 
 * Initializes pointers to null. The actual scene setup
 * happens in Init().
 */
DemoScene::DemoScene()
{
	// Default to 1920x1080 (logical FHD). Resize() (or the boot-time
	// glfwGetFramebufferSize() call in glfwApp::Init()) overrides these
	// with the real framebuffer pixel size before the FBO is created.
	m_view_width = 1920;
	m_view_height = 1080;


	m_scenemanager = nullptr;
	m_viewer_scene = nullptr;

	m_playerRig = nullptr;
	m_speaker = nullptr; // Initialize m_speaker
}

DemoScene::~DemoScene()
{
	if( m_pipeline ) { m_vortxge->setRenderPipeline( nullptr ); delete m_pipeline; }
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

/**
 * @brief Initialize demo scene
 * 
 * This is the main initialization function that sets up:
 * 1. Creates vortxGE engine instance
 * 2. Initializes GLEW for OpenGL extension handling
 * 3. Enables OpenGL debug output
 * 4. Initializes the vortxGE engine
 * 5. Sets OpenGL default state (face culling)
 * 6. Sets up user controls
 * 7. Creates the test scene
 * 
 * @return true if initialization successful, false otherwise
 * 
 * @note Must be called after OpenGL context is created
 * @note Must be called before any rendering
 */

bool DemoScene::Init()
{
	cout << "DemoScene::Init() called" << endl;
	
	// create m_vortxge - must have an active GL context (OpenGL backend)
	// or a GLFW_NO_API window (Vulkan backend)
	m_vortxge = new vortxGE();
	if( m_vortxge )
	{
		// Resolve the effective backend: explicit preference (from the
		// host app) wins; otherwise engine env/probe logic decides.
		RenderType rt = (m_prefRenderer != RenderType::None)
			? m_prefRenderer : vortxGE::PreferredRendererType();
		m_vortxge->ForceRenderer( rt );
		m_activeRenderer = rt;
		m_vortxge->SetWindow( m_prefWindow );

		if( rt == RenderType::OpenGL )
		{
			// start GLEW extension handler
			glewExperimental = GL_TRUE;
			glewInit();

			// Need debug context and extensions loaded first.
			glEnable( GL_DEBUG_OUTPUT );
			glDebugMessageCallback( MessageCallback, 0 );
		}

		m_vortxge->Init();

		if( rt == RenderType::OpenGL )
		{
			// Set some openGL defaults
			glFrontFace( GL_CCW );
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
		}
		
		setupControls();
		setupTestScene();

		// The stage-based pipeline is THE render path (R6, 2026-08-14 —
		// the legacy pass-manager/viewer chain has been removed).
		initPipeline();
		
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

	// Remember the framebuffer pixel dimensions so subsequent code
	// (notably the main-scene FBO at the bottom of setupTestScene) is
	// sized correctly. The GLFW front-end calls this with the result of
	// glfwGetFramebufferSize() so we always get physical pixels, not
	// logical units that may differ by the desktop scale factor.
	m_view_width  = w;
	m_view_height = h;

	m_vortxge->Resize( x, y, w, h );

	if( m_pipeline ) m_pipeline->setFramebufferSize( w, h );
}

// ************************************
//  Controls
// ************************************

void DemoScene::setupControls()
{
	cout << "DemoScene::setupControls() called" << endl;

	
	
	// R6 (2026-08-14): the demo owns its scene manager directly — the
	// lsViewer/lsControl render chain has been removed.
	m_scenemanager = new lsSceneManager();
	if( m_scenemanager )
	{

		// Create viewer scene
		// World size 1000 meters
		// maxProxies - Set this high enough for your physics objects or they wont be added to the sim.
		m_viewer_scene = new lsScene( m_vortxge->getRender(), 1000.0, 1000 );
		if( m_viewer_scene )
		{
			m_scenemanager->Add_Scene( m_viewer_scene );
			
			//***********************************************
			// Add Player Rig - instantiate lsFirstPersonRig
			m_playerRig = new lsFirstPersonRig(m_viewer_scene, m_viewer_scene->getBulletworld()->getDynamicsWorld());
			m_playerRig->Set_Pos(0, 5, 8); // Position camera to look at the scene from +Z
			m_viewer_scene->Add_Node(m_playerRig);
			
			lsCamera* playerCam = m_playerRig->getCamera();
			playerCam->Set_Perspective(45.0, 1820.0 / 1080.0, 0.1, 1000.0);
			// E3.2/R5: the scene owns the active camera; pipeline stages
			// read it from the scene, not from the rig/control classes.
			m_viewer_scene->setActiveCamera( playerCam );
			
			
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
		// L4.1 PCSS: per-light penumbra sizes (uv units of the shadow
		// map). Sun: small (distant light, huge 400m ortho); spots:
		// larger so the soft-edge effect is clearly visible in the demo.
		for( lsLight* l : m_viewer_scene->getLightManager()->getLights() )
		{			if( l->getLight_type() == LSLT_DIRECT ) l->shadow_softness = 0.004f;
			else if( l->getLight_type() == LSLT_SPOT ) l->shadow_softness = 0.03f;
		}

		// L3.2 dev light: VORTX_POINTLIGHT=1 adds a shadow-casting
		// point light near the objects (and frees a shader slot by
		// deactivating the second spot — the shaders take 3 lights).
		if( std::getenv( "VORTX_POINTLIGHT" ) )
		{
			lsLight_Point* pl = new lsLight_Point( m_viewer_scene );
			pl->setName( "Point Light" );
			pl->Set_Pos( 0.0, 4.0, 0.0 );
			pl->diffuse = lsVector4( 1.0, 0.8, 0.6, 1.0 );
			pl->intensity = 30.0f;
			pl->point_shadow_near = 0.1f;
			pl->point_shadow_far = 30.0f;
			m_viewer_scene->Add_Node( pl );
			m_viewer_scene->getLightManager()->Add_Light( pl );
			// Del (not setActive): setActiveLights re-activates
			// everything in range each frame, which would push the
			// point light past the 3 shader slots again.
			for( lsLight* l : m_viewer_scene->getLightManager()->getLights() )
				if( l->getLight_type() == LSLT_SPOT &&
					l->getName() == "Spot Light" )
				{
					m_viewer_scene->getLightManager()->Del_Light( l );
					break;
				}
		}
		}// if viewer_scene
	}// if viewer
}


// ************************************
//  Test Scene
// ************************************

void DemoScene::setupTestScene()
{
	// Tech-demo scene selection: VORTX_SCENE=<name> loads data/<name>.vgs
	// (exported from demos/scenes_src/<name>.blend via tools/export_scenes.py).
	// Default "demoscene" = the base demo. The base scene keeps its
	// procedural extras (object emitter, Khronos sample models); tech
	// scenes are pure .vgs content plus the shared environment shell
	// (skybox, physics ground, speaker).
	const char* sceneEnv = std::getenv( "VORTX_SCENE" );
	m_sceneName = ( sceneEnv && *sceneEnv ) ? sceneEnv : "demoscene";
	m_liveLink = std::getenv( "VORTX_LIVELINK" ) != nullptr;
	m_physCount = std::getenv( "VORTX_PHYSICS_COUNT" ) != nullptr;
	if( m_liveLink )
		cout << "INFO: live-link enabled — watching data/" << m_sceneName << ".vgs" << endl;
	const bool baseScene = ( m_sceneName == "demoscene" );
	cout << "DemoScene: scene '" << m_sceneName << "'" << endl;

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
	
	// Load the selected scene file
	if( m_scenemanager )
	{
		const std::string sceneFile = "data/" + m_sceneName + ".vgs";
		if( !m_scenemanager->LoadSceneFromFile( sceneFile, m_viewer_scene ) )
			cerr << "ERROR: failed to load scene file " << sceneFile << endl;
		
		// Manually trigger a single Move pass to update ModelMatrix for all nodes
		// This is necessary so that nodes have their correct world positions for light linking.
		m_viewer_scene->Move(m_viewer_scene, m_vortxge->getRender(), 0.0); // Update scene's own matrix
		m_viewer_scene->MoveThread(m_viewer_scene, m_vortxge->getRender(), 0.0); // Update all children recursively
		
		// After loading and updating transforms, link lights to objects
		m_scenemanager->LinkLightsInScene(m_viewer_scene, 64); // L5.2
	}

	// M4: register mesh objects for distance-driven streaming.
	// Excluded: the skybox (must never unload) and the emitter's shared
	// mesh (ref-locked by spawned clones anyway). Evicted objects simply
	// don't draw until the player comes back within loadRadius.
	{
		lsResidencyConfig streamCfg;
		streamCfg.loadRadius = 50.0f;
		streamCfg.unloadRadius = 80.0f;
		streamCfg.maxResident = 16;
		m_streamingManager.config() = streamCfg;
		m_streamingManager.setLoader( &m_asyncLoader );	// M5: async loads

		std::function<void(lsNode*)> walk = [&]( lsNode* n ) {
			for (auto& child : n->getChildren()) {
				lsObject* obj = dynamic_cast<lsObject*>( child.get() );
				if (obj && obj != skyObject && obj->getAsset()
					&& obj->getAsset()->getAsset_type() == LSAT_MESH)
				{
					m_streamingManager.registerNode( obj, obj->getAsset() );
				}
				walk( child.get() );
			}
		};
		walk( m_viewer_scene );
		cout << "INFO: DemoScene streaming enabled ("
			 << m_streamingManager.entries().size() << " objects, load < "
			 << streamCfg.loadRadius << ", evict > " << streamCfg.unloadRadius
			 << ")" << endl;
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
	
	// Base-demo content only (tech scenes are pure .vgs)
	if( baseScene )
	{
		// Create object emitter.
		setupObjectEmitter();

		// Khronos sample models (data/testmodels/)
		setupSampleModels();
	}

	// Debug-gated ad-hoc test model (VORTX_TEST_MODEL) — any scene
	setupTestModel();
}

// -------------------------------------------------------------------
// D-series diagnostic: load an ad-hoc glTF/.glb model into the scene
// for validation beyond the demo assets (e.g. Khronos sample models).
//   VORTX_TEST_MODEL=path.glb[:x,y,z[:scale]]
// Compiled out entirely in Release (no env string, no code).
// -------------------------------------------------------------------
void DemoScene::setupTestModel()
{
#if defined(VORTX_DEBUG_TOOLS)
	const char* e = getenv( "VORTX_TEST_MODEL" );
	if(! e || ! *e || ! m_viewer_scene ) return;

	// parse path[:x,y,z[:scale]]
	std::string spec = e;
	float px = 4.0f, py = 1.0f, pz = -6.0f, sc = 1.0f;
	std::string path = spec;
	const size_t c1 = spec.find( ':' );
	if( c1 != std::string::npos )
	{
		path = spec.substr( 0, c1 );
		sscanf( spec.c_str() + c1 + 1, "%f,%f,%f", &px, &py, &pz );
		const size_t c2 = spec.find( ':', c1 + 1 );
		if( c2 != std::string::npos )
			sc = (float)atof( spec.c_str() + c2 + 1 );
	}

	lsAsset_Mesh* mesh = m_viewer_scene->getAssetManager()
						 ->CreateAsset_Mesh( path.c_str() );
	if(! mesh )
	{
		cout << "VORTX_TEST_MODEL: FAILED to load " << path << endl;
		return;
	}
	mesh->AllocAsset();

	lsObject* obj = new lsObject( m_viewer_scene );
	obj->setName( "TestModel" );
	obj->Set_Asset( mesh );
	obj->Set_Scale( sc, sc, sc );
	obj->Set_Pos( px, py, pz );
	obj->Set_Rot( 0, 0, 0 );
	m_viewer_scene->Add_Node( obj );
	cout << "VORTX_TEST_MODEL: loaded " << path << " at ("
		 << px << "," << py << "," << pz << ") scale " << sc << endl;
#endif
}

// -------------------------------------------------------------------
// Khronos glTF sample models (demos/data/testmodels/ — CC0 / CC-BY,
// see ATTRIBUTION.md there). Permanent scene content: real-world PBR
// meshes exercising the light_tex_* shaders beyond the demo assets.
// NOTE: non-Blender-export glTF materials currently render dark on
// both backends (known material/shader variant gap, see commit
// 9243f36 message) — the models are still useful for geometry,
// shadow and loader validation.
// -------------------------------------------------------------------
void DemoScene::setupSampleModels()
{
	if (!m_viewer_scene) return;

	struct SampleDef { const char* file; const char* name;
					   float x, y, z, s; };
	const SampleDef defs[] = {
		{ "data/testmodels/WaterBottle.glb", "SampleWaterBottle",
		  -1.1f, 1.15f, -3.2f, 6.0f },	// on the crate's left corner
		{ "data/testmodels/Corset.glb", "SampleCorset",
		   2.6f, 0.85f, -5.0f, 1.2f },	// mannequin, right of crate
		{ "data/testmodels/AlphaBlendModeTest.glb", "SampleAlphaBlend",
		   6.0f, -1.0f, -10.0f, 0.6f },	// alpha-test grid, back right
	};
	for (const auto& d : defs)
	{
		lsAsset_Mesh* mesh = m_viewer_scene->getAssetManager()
							 ->CreateAsset_Mesh( d.file );
		if (!mesh)
		{
			cout << "WARNING: sample model missing: " << d.file << endl;
			continue;
		}
		mesh->AllocAsset();

		lsObject* obj = new lsObject( m_viewer_scene );
		obj->setName( d.name );
		obj->Set_Asset( mesh );
		obj->Set_Scale( d.s, d.s, d.s );
		obj->Set_Pos( d.x, d.y, d.z );
		obj->Set_Rot( 0, 0, 0 );
		m_viewer_scene->Add_Node( obj );
	}
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
#if defined(VORTX_DEBUG_TOOLS)
	// D2 diagnostic: deterministic captures (VORTX_FRAME_DUMP) need a
	// deterministic scene. The emitter schedules spawns by WALL CLOCK
	// (lsTimer) and lsObject lifetimes are wall-clock too, so spawning
	// is disabled for fixed-dt runs; the static scene is identical
	// frame-for-frame across runs and backends.
	if( getenv( "VORTX_FIXED_DT" ) )
	{
		emitter->setEmittime(1e12);
		cout << "DemoScene: VORTX_FIXED_DT set — emitter spawning disabled" << endl;
	}
#endif
	emitter->setEmit_life(20.0 * 1000.0); // Life of 20 seconds
	emitter->setMass(50.0); // 50kg
	
	// Set randomization properties for more interesting emission
	emitter->setRand_area(lsVector3(2.0f, 0.0f, 2.0f)); // Random area around emitter

	// Object pool (emitter redesign): pre-allocate everything now —
	// nothing is created/deleted at spawn time (VRAM churn -> RADV crash)
	emitter->setPoolSize( 100 );
	emitter->PreAllocate( m_viewer_scene, m_vortxge->getRender() );
	emitter->setRand_velocity(lsVector3(2.0f, 5.0f, 2.0f)); // Random velocity
	emitter->setRand_rotation(true); // Random rotation
	
	// Add lights
	emitter->add_Emit_Light(m_sun_light);
	
	// Add the emitter to the scene
	m_viewer_scene->Add_Node(emitter);
	
	cout << "DemoScene::setupObjectEmitter() completed" << endl;
}


void DemoScene::Update( double delta )
{
	VORTX_PROFILE_ZONE( "update" );

	// Physics body count + fps every 5s (VORTX_PHYSICS_COUNT=1).
	// Proves the emitter pool keeps the Bullet world bounded (E1.1).
	// NOTE: delta is in MILLISECONDS (glfwApp main loop).
	if( m_physCount )
	{
		m_physAcc += delta;
		m_physFrames++;
		if( m_physAcc >= 5000.0 )
		{
			int bodies = -1;
			if( m_viewer_scene && m_viewer_scene->getBulletworld()
				&& m_viewer_scene->getBulletworld()->getDynamicsWorld() )
				bodies = m_viewer_scene->getBulletworld()->getDynamicsWorld()->getNumCollisionObjects();
			cout << "PHYSICS-COUNT: bodies=" << bodies
			     << " fps=" << ( m_physFrames / ( m_physAcc / 1000.0 ) ) << endl;
			m_physAcc = 0.0;
			m_physFrames = 0;
		}
	}

	// Live-link: poll the scene file mtime twice a second; on change
	// re-apply transforms in place (no structural changes, pipeline-
	// safe) and refresh world matrices.
	if( m_liveLink && m_scenemanager )
	{
		m_liveCheckAcc += delta;
		if( m_liveCheckAcc >= 0.5 )
		{
			m_liveCheckAcc = 0.0;
			const std::string sceneFile = "data/" + m_sceneName + ".vgs";
			const std::string fullpath = lsDir().getexepath() + sceneFile;
			struct stat st;
			if( ::stat( fullpath.c_str(), &st ) == 0 )
			{
				if( m_liveMtime == 0 ) m_liveMtime = (long)st.st_mtime;
				else if( (long)st.st_mtime != m_liveMtime )
				{
					m_liveMtime = (long)st.st_mtime;
					int n = m_scenemanager->ApplySceneTransforms(
						sceneFile, m_viewer_scene );
					// refresh world matrices so rendering + shadow
					// stages see the new transforms this frame
					m_viewer_scene->Move( m_viewer_scene, m_vortxge->getRender(), 0.0 );
					m_viewer_scene->MoveThread( m_viewer_scene, m_vortxge->getRender(), 0.0 );
					cout << "INFO: live-link applied " << n << " node update(s)" << endl;
				}
			}
		}
	}

	lsEvent *e;
	
	// process scene events
	while( (e = m_viewer_scene->Poll_Event()) )
	{
		processEvent( e );
	}
	
	// m_vortxge update
	if( m_vortxge )
	{
		// R6: scenes tick directly via the demo-owned scene manager
		// (was: controlmanager -> lsViewer::Update -> scene chain).
		if( m_scenemanager )
			m_scenemanager->Update( m_vortxge->getRender(), delta );
		m_vortxge->Update( delta );
	}

	// M3/M5: drive asset streaming exactly once per frame, on this (main/GL)
	// thread, with the viewer camera only. (The old lsScene::Optimize hook
	// was also fed shadow-pass cull cameras, causing load/evict flapping.)
	if( m_playerRig && m_playerRig->getCamera() )
	{
		const lsMatrix4x4& cm = m_playerRig->getCamera()->ModelMatrix;
		m_streamingManager.update( lsVector3( cm.matrix[12], cm.matrix[13], cm.matrix[14] ) );
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


void DemoScene::initPipeline()
{
	cout << "DemoScene::initPipeline() - Plan.md Part 3 (R2)" << endl;

	lsRender* r = m_vortxge->getRender();
	m_pipeline = new lsRenderPipeline( r );
	m_pipeline->setFramebufferSize( m_view_width, m_view_height );

	// named target for the main scene (colour + depth)
	m_pipeline->targets().acquire( "scene", true );

	// stage 0: shadow maps (Plan.md Part 3 R4 pulled forward — the PBR
	// shaders sample them; without shadow stages every lit surface is
	// black). Each stage owns its depth FBO and self-wires it to the
	// light, replacing DemoScene::setupShadows in pipeline mode.
	const lsShadowQuality shadowQ = lsShadowMap::getQuality();
	// L2.1b dev flag: VORTX_CSM=1 replaces the sun's single-map stage
	// with a 4-cascade CSM stage (atlas). Shading still uses the legacy
	// single-map path until L2.4 — sun shadows look wrong with the flag
	// on; verify atlas content via VORTX_DEPTH_SHOT instead.
	const bool useCsm = std::getenv( "VORTX_CSM" ) != nullptr;
	for( lsLight* light : m_viewer_scene->getLightManager()->getLights() )
	{
		// L5.3: non-casting lights get no shadow stage at all (the
		// stress scene's 64 point lights must not spawn 64 stages)
		if( !light->cast_shadows ) continue;
		if( light->getLight_type() == LSLT_POINT )
		{
			// L3.2: omnidirectional shadows — cube depth target, 6-face
			// pass. 1024/face at High (6x1024^2 ~ one 2048 2D map).
			// (before the resolutionFor guard: it returns 0 for points)
			const int ms = shadowQ == lsShadowQuality::High ? 1024 :
						   shadowQ == lsShadowQuality::Medium ? 512 : 256;
			auto* ps = m_pipeline->addStage<lsPointShadowStage>(
				m_viewer_scene, light, ms, r );
			ps->setName( string( "pointshadow-" ) + light->getName() );
			continue;
		}
		if( lsShadowMap::resolutionFor( light, shadowQ ) <= 0 ) continue;
		if( useCsm && light->getLight_type() == LSLT_DIRECT )
		{
			const int ms = lsShadowMap::resolutionFor( light, shadowQ ) / 2;
			auto* cs = m_pipeline->addStage<lsCsmStage>(
				m_viewer_scene, light, m_viewer_scene->getActiveCamera(), ms, r );
			cs->setName( string( "csm-" ) + light->getName() );
			continue;
		}
		auto* sh = m_pipeline->addStage<lsShadowStage>(
			m_viewer_scene, light, shadowQ, r );
		sh->setName( string( "shadow-" ) + light->getName() );
	}

	// L5: cluster light assignment (builds + uploads the grid before
	// the main pass; shaders read it via the scene)
	auto* cl = m_pipeline->addStage<lsClusterStage>(
		m_viewer_scene, m_viewer_scene->getActiveCamera(), r );
	cl->setName( "cluster-lights" );

	// L4.1b: depth+normal prepass before the main pass (High preset
	// only — it feeds L4.2 contact shadows, which are High-gated).
	// Doubles geometry load; skip entirely on lower presets.
	if( shadowQ == lsShadowQuality::High )
	{
		m_pipeline->targets().acquire( "prepass", true );
		auto* pp = m_pipeline->addStage<lsPrepassStage>(
			m_viewer_scene, m_viewer_scene->getActiveCamera(), "prepass" );
		pp->setName( "prepass" );
	}

	// stage 1: main scene -> "scene" target
	auto* scene = m_pipeline->addStage<lsSceneStage>(
		m_viewer_scene, m_viewer_scene->getActiveCamera(), "scene" );
	scene->setName( "main-scene" );

	// stage 2: "scene" target -> screen (replaces the Quad Viewer)
	auto* blit = m_pipeline->addStage<lsBlitStage>( "scene", "" );
	blit->setName( "blit-to-screen" );
	// Vulkan's negative-height viewport stores FBO texels V-mirrored vs
	// GL — compensate when sampling (research: VK_KHR_maintenance1
	// viewport-flip convention, 'offscreen rendering must be
	// compensated'). GL needs no flip. X is never affected.
	blit->setFlipY( m_activeRenderer == RenderType::Vulkan );

	m_vortxge->setRenderPipeline( m_pipeline );
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

