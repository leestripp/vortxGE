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
// terrain module (engine feature)
#include <terrain/Heightfield.h>
#include <terrain/Noise.h>
#include <terrain/Erosion.h>
#include <terrain/Biome.h>
#include <terrain/Mesh.h>
#include <terrain/TerrainMesh.h>
#include <terrain/TextureGen.h>
#include <terrain/veg/Scatterer.h>
#include <terrain/veg/VegetationLayer.h>
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
#include <scene/lsFlyCam.h>
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

	// Sky setup — S-series procedural sky with day/night cycle.
	// VORTX_TIME_OF_DAY=<hours> sets the start time (default 10:00,
	// matching the previous fixed sun); VORTX_DAY_LENGTH=<seconds>
	// animates the cycle (default 0 = frozen, capture-deterministic).
	{
		m_sky = new lsSky();
		if( const char* t = std::getenv( "VORTX_TIME_OF_DAY" ) )
			m_sky->setTimeOfDay( (float)atof( t ) );
		if( const char* d = std::getenv( "VORTX_DAY_LENGTH" ) )
			m_sky->setDayLength( (float)atof( d ) );
		if( m_sky->init( m_viewer_scene, m_vortxge->getShaderManager(),
						 m_vortxge->getRender() ) )
		{
			m_sky->attachSunLight( m_sun_light );
			m_sky->apply();
			cout << "DemoScene: procedural sky, t=" << m_sky->getTimeOfDay()
				 << "h dayLength=" << m_sky->getDayLength() << "s" << endl;
		} else
		{
			std::cerr << "ERROR: lsSky init failed — no sky." << std::endl;
			delete m_sky; m_sky = nullptr;
		}
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
		m_scenemanager->LinkLightsInScene(m_viewer_scene, 128); // L5.2
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
				if (obj && obj != m_viewer_scene->getSkyObject() && obj->getAsset()
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

		if( getenv( "VORTX_TERRAIN" ) )
			setupTerrain();
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

// Terrain chunk registry for the per-frame LOD pass (setupTerrain fills
// it; Update_TerrainLod swaps each chunk's asset to the best stride).
namespace {
struct TerrainChunkRec {
	lsObject* obj = nullptr;
	lsAsset_Mesh* assets[4] = { nullptr, nullptr, nullptr, nullptr };
	lsVector3 centre;
	float radius = 0.0f;
	int lod = 0;
};
struct VegChunkRec {            // distance-culled via instanceDrawCount
	lsBatch_Mesh* batch = nullptr;
	lsVector3 centre;           // WORLD position of the chunk centre
	float radius = 0.0f;
	float maxDist = 0.0f;       // 0 = always draw
	int fullCount = 0;          // instances at LOD0
	bool lodThin = false;       // grass only: ramp instance count
};
}
static std::vector<TerrainChunkRec> s_terrainChunks;
static std::vector<VegChunkRec> s_vegChunks;

// -------------------------------------------------------------------
// Terrain demo (VORTX_TERRAIN=1): procedural heightfield (noise ->
// erosion -> biomes) rendered as an engine mesh + Bullet heightfield
// collision + scattered vegetation. Placed east of the built area,
// centre at (kCX, 0, kCZ); the player rig spawns on it.
// -------------------------------------------------------------------
// flycam default pose: above terrain centre looking north-ish
static constexpr float kFlyDefX = 700.0f, kFlyDefY = 120.0f, kFlyDefZ = 0.0f;

// The demo's veg OBJs were authored growing along -Z (Blender export
// without the Y-up conversion): tree trunk runs 0 -> -3.2 in Z, grass
// blades lie flat pointing -Z. Rotate +90 deg about X so -Z becomes
// +Y (upright): (x,y,z) -> (x,-z,y). Pure rotation preserves winding.
static void uprightMeshZneg( terrain::Mesh& mesh )
{
	for( auto& v : mesh.vertices )
	{
		const float y = v.py, z = v.pz;
		v.py = -z; v.pz = y;
		const float ny = v.ny, nz = v.nz;
		v.ny = -nz; v.nz = ny;
	}
	const float mnY = mesh.aabbMin[1], mxY = mesh.aabbMax[1];
	const float mnZ = mesh.aabbMin[2], mxZ = mesh.aabbMax[2];
	mesh.aabbMin[1] = -mxZ; mesh.aabbMax[1] = -mnZ;
	mesh.aabbMin[2] = mnY;  mesh.aabbMax[2] = mxY;
}

void DemoScene::setupTerrain()
{
	if( ! m_viewer_scene ) return;
	lsRender* render = m_vortxge->getRender();

	constexpr int kRes = 256;
	constexpr float kWorld = 1024.0f;
	constexpr float kHeight = 180.0f;
	constexpr float kCX = 700.0f;   // terrain centre, east of the demo area
	constexpr float kCZ = 0.0f;

	// -- generate: fBm + ridged base, hydraulic erosion -----------------
	terrain::Noise noise( 1337 );
	terrain::Heightfield hf( kRes, kRes );
	for( int y = 0; y < kRes; ++y )
		for( int x = 0; x < kRes; ++x )
		{
			const float u = ( float )x / ( kRes - 1 );
			const float v = ( float )y / ( kRes - 1 );
			const float base = noise.fbm( u * 4.0f, v * 4.0f, 6 ) * 0.5f + 0.5f;
			const float ridge = noise.ridged( u * 3.0f + 7.3f, v * 3.0f + 2.1f, 5 );
			hf( x, y ) = base * 0.65f + ridge * 0.45f;
		}
	hf.normalize();
	terrain::ErosionParams ep;
	ep.seed = 42;
	ep.droplets = 60000;
	terrain::erode( hf, ep );
	hf.normalize();
	cout << "VORTX_TERRAIN: heightfield " << kRes << "x" << kRes
		 << " range [" << hf.minValue() << ", " << hf.maxValue() << "]" << endl;

	// -- render meshes: 8x8 chunks x 4 LOD strides --------------------
	// Per-chunk tight bSpheres make the engine frustum-cull unseen
	// chunks (lsObject::Optimize) and the per-frame LOD pass below
	// swaps each chunk to the coarsest adequate stride for its
	// distance from the camera.
	lsShaderManager* shaders = m_vortxge->getShaderManager();
	// Plan.md T1: terrain_splat = light_tex_none + 4 procedural splat
	// layers. setKind is REQUIRED after Load_Shader: the kind selects
	// which sampler uniforms get texture units in UseShader (the
	// default kind=None silently leaves all samplers on unit 0).
	lsShader* terrainShader = shaders->Find_Shader( "terrain_splat" );
	if( ! terrainShader )
	{
		terrainShader = new lsShader( render );
		terrainShader->Load_Shader( "terrain_splat" );
		terrainShader->setKind( ShaderKind::LightTexNone );
		shaders->Add_Shader( terrainShader );
	}

	// Plan.md T2: procedural splat layers (albedo RGB + height A),
	// uploaded once and shared by all chunk batches on units 14-17.
	lsTextureID splatTex[4] = { 0, 0, 0, 0 };
	if( ! std::getenv( "VORTX_NOTEXTURE" ) )
	{
		for( int i = 0; i < 4; ++i )
		{
			terrain::SplatLayer layer = terrain::generateSplatLayer( i );
			render->GenTextures( 1, &splatTex[i] );
			render->BindTexture( LS_TEXTURE_2D, splatTex[i] );
			render->TexImage2D( LS_TEXTURE_2D, 0, LS_RGBA8,
								layer.size, layer.size, 0, LS_RGBA,
								LS_UNSIGNED_BYTE, layer.rgba.data() );
			render->GenerateMipmap( LS_TEXTURE_2D );
			render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_REPEAT );
			render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_REPEAT );
			render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER,
								   LS_LINEAR_MIPMAP_LINEAR );
			render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
		}
	}

	constexpr int kChunks = 8;
	constexpr int kLods = 4;
	const int kStride[kLods] = { 1, 2, 4, 8 };
	constexpr float kSkirt = 6.0f; // metres, hides LOD T-junctions

	s_terrainChunks.clear();
	s_terrainChunks.reserve( kChunks * kChunks );
	for( int cy = 0; cy < kChunks; ++cy )
		for( int cx = 0; cx < kChunks; ++cx )
		{
			TerrainChunkRec rec;
			rec.lod = 0;
			for( int l = 0; l < kLods; ++l )
			{
				terrain::GridMesh cg = terrain::generateChunkGridMesh(
					hf, kWorld, kHeight, cx, cy, kChunks, kStride[l], kSkirt );
				lsMesh* m = terrain::uploadGridMesh( cg, render, terrainShader );
				if( ! m ) { cerr << "VORTX_TERRAIN: chunk upload failed" << endl; return; }
				for( lsBatch* b = m->getBatch_list(); b; b = b->getNext_batch() )
					for( int i = 0; i < 4; ++i )
						static_cast<lsBatch_Mesh*>( b )->tex_splat[i] = splatTex[i];
				rec.assets[l] = new lsAsset_Mesh( render );
				rec.assets[l]->setData( m ); // generated mesh; AllocAsset is for files
			}
			rec.obj = new lsObject( m_viewer_scene );
			rec.obj->setName( "TerrainChunk" );
			rec.obj->Set_Asset( rec.assets[0] );
			rec.obj->Set_Pos( kCX, 0, kCZ );
			m_viewer_scene->Add_Node( rec.obj );

			// world-space cull/LOD data from the full-res mesh bSphere
			lsMesh* m0 = static_cast<lsMesh*>( rec.assets[0]->getData() );
			rec.centre = lsVector3( kCX + m0->getBSphere().center.x,
			                        m0->getBSphere().center.y,
			                        kCZ + m0->getBSphere().center.z );
			rec.radius = m0->getBSphere().radius;
			s_terrainChunks.push_back( rec );
		}
	cout << "VORTX_TERRAIN: " << s_terrainChunks.size() << " chunks x "
		 << kLods << " LODs, skirt " << kSkirt << "m" << endl;

	// -- Bullet heightfield collision (static) ---------------------------
	// btHeightfieldTerrainShape is centre-origin in ALL axes (see
	// lsBulletShape::Make_Heightfield): lift the body by half the max
	// height so the collision surface aligns with the render mesh.
	std::vector<float> heights( ( size_t )kRes * kRes );
	for( int y = 0; y < kRes; ++y )
		for( int x = 0; x < kRes; ++x )
			heights[( size_t )y * kRes + x] = hf( x, y );

	lsBulletShape* hfShape = new lsBulletShape( render );
	const float cell = kWorld / ( kRes - 1 );
	hfShape->Make_Heightfield( kRes, kRes, heights.data(), kHeight, cell, cell );

	lsObject* terrainBody = new lsObject( m_viewer_scene );
	terrainBody->setName( "TerrainBody" );
	terrainBody->Set_Pos( kCX, kHeight * 0.5f, kCZ );
	lsBulletObject* terrainBullet = new lsBulletObject(
		terrainBody, hfShape->getShape(),
		m_viewer_scene->getBulletworld()->getMax_proxies() );
	terrainBullet->Init( 0.0f ); // static
	terrainBody->setBulletObject( terrainBullet );
	m_viewer_scene->Add_Node( terrainBody );
	cout << "VORTX_TERRAIN: heightfield collision at (" << kCX << ",0," << kCZ
		 << "), cell " << cell << "m, height x" << kHeight << endl;

	// -- vegetation: grass + bushes + trees, GPU-INSTANCED ------------
	if( ! std::getenv( "VORTX_NOVEG" ) ) {
	// One lsObject per (chunk, species): the species mesh draws once
	// per chunk with per-instance pos/rotY/scale from an instance VBO
	// (lsBatch_Mesh::setInstanceData -> attributes 5/6, divisor 1).
	// Grass adds a per-frame distance cutoff via setInstanceDrawCount.
	terrain::Noise climate( 9001 );
	std::vector<terrain::Biome> biomes( ( size_t )kRes * kRes );
	for( int y = 0; y < kRes; ++y )
		for( int x = 0; x < kRes; ++x )
		{
			const float u = ( float )x / ( kRes - 1 );
			const float v = ( float )y / ( kRes - 1 );
			const float h = hf( x, y );
			const float temp = std::clamp(
				0.75f - h * 0.55f + climate.fbm( u * 2.0f, v * 2.0f, 3 ) * 0.08f,
				0.0f, 1.0f );
			const float moist = std::clamp(
				0.5f + climate.fbm( u * 3.0f + 11.0f, v * 3.0f + 5.0f, 4 ) * 0.6f,
				0.0f, 1.0f );
			// sea level 0.08: only the deepest erosion channels count as
			// ocean — this map renders no water, so the classifier's
			// default 0.30 band would eat the walkable lowlands and
			// starve every vegetation mask there (the "one patch" bug).
			biomes[( size_t )y * kRes + x] =
				terrain::classifyBiome( h, temp, moist, 0.08f );
		}

	// biome histogram: proves the sea-level rebalance actually covers
	// the lowlands (was: everything Ocean/Beach = the "one patch" bug)
	{
		int counts[( size_t )terrain::Biome::Count] = {};
		for( auto b : biomes ) counts[( size_t )b]++;
		cout << "VORTX_TERRAIN: biomes";
		for( int i = 0; i < ( int )terrain::Biome::Count; ++i )
			cout << " " << terrain::biomeName( ( terrain::Biome )i )
				 << "=" << ( 100 * counts[i] / ( kRes * kRes ) ) << "%";
		cout << endl;
	}

	terrain::veg::Scatterer scatterer( hf, biomes, kWorld, kHeight );

	// instancing-aware shaders (batch Draw picks shadowmap_inst by name)
	lsShader* instShader = shaders->Find_Shader( "light_tex_none_inst" );
	if( ! instShader )
	{
		instShader = new lsShader( render );
		instShader->Load_Shader( "light_tex_none_inst" );
		// setKind() resolves + assigns the sampler units; loading by
		// name leaves kind None -> samplers default to unit 0 ->
		// "samplers of different type share a unit" validation error.
		instShader->setKind( ShaderKind::LightTexNone );
		shaders->Add_Shader( instShader );
	}
	if( ! shaders->Find_Shader( "shadowmap_inst" ) )
	{
		lsShader* s = new lsShader( render );
		s->Load_Shader( "shadowmap_inst" );
		s->setKind( ShaderKind::ShadowMap );
		shaders->Add_Shader( s );
	}

	struct VegDef { const char* name; const char* file; int capPerChunk;
					float minAlt, maxAlt, slope, spacing, density;
					unsigned biomeMask; bool distanceCull;
					float r, g, b;
					float scaleMul;  // world-size target / mesh size
					};
	const unsigned forestMask = terrain::biomeBit( terrain::Biome::Forest )
							| terrain::biomeBit( terrain::Biome::Grassland );
	const VegDef vegDefs[] = {
		{ "Grass", "data/vegetation/grass_clump.obj", 12000,
		  0.05f, 0.85f, 34.0f, 1.0f, 1.0f,
		  forestMask | terrain::biomeBit( terrain::Biome::Savanna ), true,
		  0.42f, 0.66f, 0.20f, 0.40f },
		{ "Bush", "data/vegetation/bush.obj", 400,
		  0.05f, 0.75f, 28.0f, 3.0f, 0.7f,
		  forestMask | terrain::biomeBit( terrain::Biome::Taiga ), false,
		  0.20f, 0.45f, 0.16f, 1.6f },
		{ "Tree", "data/vegetation/broadleaf.obj", 200,
		  0.05f, 0.82f, 30.0f, 9.0f, 0.8f,
		  forestMask | terrain::biomeBit( terrain::Biome::Taiga ), false,
		  0.26f, 0.48f, 0.18f, 2.8f },
	};

	static const bool vegDebug = std::getenv( "VORTX_VEG_DEBUG" ) != nullptr;
	s_vegChunks.clear();
	for( const auto& vd : vegDefs )
	{
		auto meshRes = terrain::loadObj( vd.file );
		if( ! meshRes )
		{
			cerr << "VORTX_TERRAIN: " << meshRes.error() << endl;
			continue;
		}
		terrain::Mesh upright = *meshRes;
		uprightMeshZneg( upright );
		const terrain::Mesh& srcMesh = upright;

		terrain::veg::VegetationLayer layer;
		layer.id = 1;
		layer.name = vd.name;
		layer.rules.minAltitude = vd.minAlt;
		layer.rules.maxAltitude = vd.maxAlt;
		layer.rules.maxSlopeDeg = vd.slope;
		layer.rules.minSpacing = vd.spacing;
		layer.rules.density = vd.density;
		layer.rules.biomeMask = vd.biomeMask;

		int objs = 0;
		long total = 0;
		float vegAggMn[3] = { 1e30f, 1e30f, 1e30f };
		float vegAggMx[3] = { -1e30f, -1e30f, -1e30f };
		for( int cy = 0; cy < kChunks; ++cy )
			for( int cx = 0; cx < kChunks; ++cx )
			{
				auto instances = scatterer.scatter( layer, cx, cy, kChunks, 7 );
				if( ( int )instances.size() > vd.capPerChunk )
				{
					// resize() would keep scan-order prefix = all instances
					// clustered in one corner of the chunk. Thin by hash
					// instead: deterministic AND spatially uniform.
					std::vector<terrain::veg::Instance> kept;
					kept.reserve( vd.capPerChunk );
					const float keepP = ( float )vd.capPerChunk
									  / ( float )instances.size();
					for( size_t i = 0; i < instances.size(); ++i )
					{
						const float h = ( float )( ( i * 2654435761u )
										& 0xFFFF ) / 65535.0f;
						if( h < keepP ) kept.push_back( instances[i] );
					}
					instances.swap( kept );
				}
				if( instances.empty() ) continue;

				// pack: 8 floats/instance, terrain-local coords
				std::vector<float> packed( instances.size() * 8 );
				float mn[3] = { 1e30f, 1e30f, 1e30f };
				float mx[3] = { -1e30f, -1e30f, -1e30f };
				for( size_t i = 0; i < instances.size(); ++i )
				{
					const auto& inst = instances[i];
					const float lx = inst.x - kWorld * 0.5f;
					const float ly = inst.z;               // elevation
					const float lz = inst.y - kWorld * 0.5f;
					// deterministic per-instance colour variation [0.8, 1.1]
					const float h1 = ( float )( ( i * 2654435761u ) & 0xFFFF )
									 / 65535.0f;
					packed[i*8+0] = lx;
					packed[i*8+1] = ly;
					packed[i*8+2] = lz;
					const float scaleMul = ( vd.distanceCull &&
						std::getenv( "VORTX_GRASS_DEBUG" ) ) ? 6.0f
						: ( vd.distanceCull && std::getenv( "VORTX_GRASS_SCALE" ) )
							? ( float )atof( std::getenv( "VORTX_GRASS_SCALE" ) )
							: vd.scaleMul;
					packed[i*8+3] = inst.scale * scaleMul;
					packed[i*8+4] = std::cos( inst.rotation );
					packed[i*8+5] = std::sin( inst.rotation );
					packed[i*8+6] = 0.8f + 0.3f * h1;
					packed[i*8+7] = 0.0f;
					if( lx < mn[0] ) mn[0] = lx; if( lx > mx[0] ) mx[0] = lx;
					if( ly < mn[1] ) mn[1] = ly; if( ly > mx[1] ) mx[1] = ly;
					if( lz < mn[2] ) mn[2] = lz; if( lz > mx[2] ) mx[2] = lz;
				}

				lsMesh* m = terrain::uploadVegetationMesh(
					srcMesh, render, instShader );
				if( ! m ) continue;
				lsBatch_Mesh* batch =
					static_cast<lsBatch_Mesh*>( m->getBatch_list() );
				batch->setInstanceData( packed.data(),
										( int )instances.size() );
				batch->m_albedo = lsVector4( vd.r, vd.g, vd.b, 1.0f );
				if( vd.distanceCull && std::getenv( "VORTX_GRASS_DEBUG" ) )
					batch->m_albedo = lsVector4( 2.0f, 0.0f, 2.0f, 1.0f );
				batch->setSkipPrepass( vd.distanceCull ||
					std::getenv( "VORTX_VEG_NOPREPASS" ) != nullptr );

			// bSphere from instance spread (NOT the tiny source mesh)
				lsBoundingSphere bs;
				bs.center = lsVector3( ( mn[0] + mx[0] ) * 0.5f,
									   ( mn[1] + mx[1] ) * 0.5f + 2.0f,
									   ( mn[2] + mx[2] ) * 0.5f );
				bs.radius = bs.center.distance(
					lsVector3( mx[0], mx[1], mx[2] ) ) + 4.0f;
				m->setBSphere( bs );

				VegChunkRec rec;
				rec.batch = batch;
				rec.centre = lsVector3( kCX + bs.center.x, bs.center.y,
										kCZ + bs.center.z );
				rec.radius = bs.radius;
				rec.maxDist = 0.0f; // set below with maxDrawDist
				rec.fullCount = ( int )instances.size();
				rec.lodThin = vd.distanceCull;
				s_vegChunks.push_back( rec );

				lsAsset_Mesh* asset = new lsAsset_Mesh( render );
				asset->setData( m );
				lsObject* veg = new lsObject( m_viewer_scene );
				veg->setName( vd.name );
				veg->Set_Asset( asset );
				veg->Set_Pos( kCX, 0, kCZ ); // instances are terrain-local
				if( vd.distanceCull || std::getenv( "VORTX_VEG_NOSHADOW" ) )
				veg->setCasts_shadows( false ); // grass + debug-all
				m_viewer_scene->Add_Node( veg );

				static const float grassDist = []()
				{
					const char* e = std::getenv( "VORTX_GRASS_DIST" );
					return e ? ( float )atof( e ) : 90.0f;
				}();
				static const float vegMaxOverride = []()
				{
					const char* e = std::getenv( "VORTX_VEG_MAXD" );
					return e ? ( float )atof( e ) : 0.0f;
				}();
				const float maxD = vegMaxOverride > 0.0f ? vegMaxOverride
					: ( vd.distanceCull ? grassDist
						: ( vd.capPerChunk <= 100 ? 800.0f : 300.0f ) );
				veg->setMaxDrawDist( maxD );
				s_vegChunks.back().maxDist = maxD;
				if( vegDebug )
				{
					cout << "VEGDBG " << vd.name << " chunk(" << cx << "," << cy
						 << ") n=" << instances.size()
						 << " local mn(" << mn[0] << "," << mn[1] << "," << mn[2]
						 << ") mx(" << mx[0] << "," << mx[1] << "," << mx[2] << ")"
						 << endl;
					for( int a = 0; a < 3; ++a )
					{
						if( mn[a] < vegAggMn[a] ) vegAggMn[a] = mn[a];
						if( mx[a] > vegAggMx[a] ) vegAggMx[a] = mx[a];
					}
				}
				++objs;
				total += ( long )instances.size();
			}
		cout << "VORTX_TERRAIN: " << vd.name << " x" << total
			 << " instanced over " << objs << " chunk draws" << endl;
		if( vegDebug )
			cout << "VEGDBG " << vd.name
				 << " AGG local mn(" << vegAggMn[0] << "," << vegAggMn[1] << ","
				 << vegAggMn[2] << ") mx(" << vegAggMx[0] << "," << vegAggMx[1]
				 << "," << vegAggMx[2] << ")" << endl;
	}

	} // VORTX_NOVEG
	// -- spawn the rig on the terrain ------------------------------------
	if( m_playerRig )
	{
		const float h0 = hf.sampleBilinear( 0.5f, 0.5f ) * kHeight;
		m_playerRig->Teleport( kCX, h0 + 3.0f, kCZ );
		cout << "VORTX_TERRAIN: rig spawn (" << kCX << "," << h0 + 3.0f
			 << "," << kCZ << ")" << endl;
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

	// S-series: advance the day/night cycle (delta is milliseconds).
	// No-op when day length is 0 (frozen — capture determinism).
	if( m_sky )
	{
		m_sky->update( delta * 0.001 );
		m_sky->apply();
	}

	// Terrain LOD pass: distance-band stride selection per chunk with
	// hysteresis (refine at 92% of the edge, coarsen at 108%) so a
	// chunk straddling a band edge doesn't flip every frame. The
	// frustum culling itself is engine-side (lsObject::Optimize tests
	// objpos + mesh bSphere) — chunking is what makes it effective.
	if( ! s_terrainChunks.empty() && m_viewer_scene
	    && m_viewer_scene->getActiveCamera() )
	{
		static const float kEdges[3] = { 192.0f, 448.0f, 960.0f };
		lsCamera* cam = m_viewer_scene->getActiveCamera();
		lsVector3 campos( cam->ModelMatrix.matrix[12],
		                        cam->ModelMatrix.matrix[13],
		                        cam->ModelMatrix.matrix[14] );
		int hist[4] = { 0, 0, 0, 0 };
		for( auto& rec : s_terrainChunks )
		{
			float d = campos.distance( rec.centre ) - rec.radius;
			if( d < 0.0f ) d = 0.0f;
			int want = rec.lod;
			while( want > 0 && d < kEdges[want - 1] * 0.92f ) --want;
			while( want < 3 && d > kEdges[want] * 1.08f ) ++want;
			if( want != rec.lod )
			{
				rec.obj->Set_Asset( rec.assets[want] );
				rec.lod = want;
			}
			hist[rec.lod]++;
		}
		// VORTX_TERRAIN_TRACE=1: prove culling + LOD correctness.
		// Visible count uses the SAME sphere/frustum test as the engine.
		static int s_lodFrame = 0;
		if( std::getenv( "VORTX_TERRAIN_TRACE" ) && ++s_lodFrame % 60 == 0 )
		{
			int vis = 0;
			for( const auto& rec : s_terrainChunks )
				if( cam->frustum.sphereInFrustum( rec.centre, rec.radius )
				    != OUTSIDE )
					++vis;
			cout << "TERRAIN_TRACE cam (" << campos.x << "," << campos.y
			     << "," << campos.z << ") chunks visible " << vis << "/"
			     << s_terrainChunks.size() << " LODs [" << hist[0] << ","
			     << hist[1] << "," << hist[2] << "," << hist[3] << "]"
			     << endl;
		}
	}

	// Vegetation LOD: per-chunk instance thinning by camera distance
	// (the engine's frustum + maxDrawDist culls whole chunks; this
	// ramps the density of the chunks that DO draw). Grass thins
	// 100/50/25% across its 90 m range; bushes/trees keep full count —
	// their hard maxDrawDist already caps them. Prefix-clamped
	// (setInstanceDrawCount) and instances are jitter-scattered, so
	// thinning is spatially uniform — no popping clusters.
	if( ! s_vegChunks.empty() && m_viewer_scene
	    && m_viewer_scene->getActiveCamera() )
	{
		lsCamera* cam = m_viewer_scene->getActiveCamera();
		lsVector3 campos( cam->ModelMatrix.matrix[12],
		                        cam->ModelMatrix.matrix[13],
		                        cam->ModelMatrix.matrix[14] );
		static int s_vegLodFrame = 0;
		const bool vegTrace = std::getenv( "VORTX_VEG_TRACE" )
						   && ++s_vegLodFrame % 60 == 0;
		int vegHist[4] = { 0, 0, 0, 0 }; // 100% / 50% / 25% / beyond
		for( auto& rec : s_vegChunks )
		{
			if( ! rec.lodThin || ! rec.batch ) continue;
			float d = campos.distance( rec.centre ) - rec.radius;
			if( d < 0.0f ) d = 0.0f;
			const float frac =
				d < rec.maxDist * 0.45f ? 1.0f :
				d < rec.maxDist * 0.75f ? 0.5f : 0.25f;
			rec.batch->setInstanceDrawCount(
				( int )( rec.fullCount * frac ) );
			if( vegTrace )
			{
				if( d >= rec.maxDist ) vegHist[3]++;
				else if( frac == 1.0f ) vegHist[0]++;
				else if( frac == 0.5f ) vegHist[1]++;
				else vegHist[2]++;
			}
		}
		if( vegTrace )
			cout << "VEG_TRACE cam (" << campos.x << "," << campos.y
				 << "," << campos.z << ") grass chunks 100%:" << vegHist[0]
				 << " 50%:" << vegHist[1] << " 25%:" << vegHist[2]
				 << " beyond:" << vegHist[3] << "/" << s_vegChunks.size()
				 << endl;
	}

#if defined(VORTX_DEBUG_TOOLS)
	// C0 debug: deterministic tiny camera orbit (amplitude ~1m) so
	// the sun-shadow bug can be reproduced without input.
	if( std::getenv( "VORTX_CAM_ORBIT" ) && m_viewer_scene
	    && m_viewer_scene->getActiveCamera() )
	{
		static double s_orbitT = 0.0;
		s_orbitT += delta * 0.0001;
		lsCamera* cam = m_viewer_scene->getActiveCamera();
		cam->Set_Pos( (float)std::sin( s_orbitT ), 5.0f,
		              8.0f + (float)std::cos( s_orbitT ) );
	}
	// S-debug: rotate the player rig through its REAL input path
	// (HandleMouseLook → Bullet yaw quaternion) to reproduce interactive
	// rotation bugs in headless captures. VORTX_RIG_YAW=<deg>,
	// VORTX_RIG_PITCH=<deg>, applied once on the first frame.
	{
		static bool s_rigRotDone = false;
		const char* ryEnv = std::getenv( "VORTX_RIG_YAW" );
		const char* rpEnv = std::getenv( "VORTX_RIG_PITCH" );
		if( ! s_rigRotDone && ( ryEnv || rpEnv ) && m_playerRig )
		{
			s_rigRotDone = true;
			// HandleMouseLook: yaw_delta = -dx * 0.05 * delta(ms);
			// pitch_delta = -dy * sens * delta. 6.0 dx @16.7ms = 5°/call.
			float yawT = ryEnv ? (float)atof( ryEnv ) : 0.0f;
			float pitchT = rpEnv ? (float)atof( rpEnv ) : 0.0f;
			int nY = (int)( std::abs( yawT ) / 5.0f + 0.5f );
			float dx = yawT > 0 ? -6.0f : 6.0f;
			for( int i = 0; i < nY; i++ )
				m_playerRig->HandleMouseLook( 16.666f, dx, 0.0f );
			int nP = (int)( std::abs( pitchT ) / 5.0f + 0.5f );
			float dy = pitchT > 0 ? -6.0f : 6.0f;
			for( int i = 0; i < nP; i++ )
				m_playerRig->HandleMouseLook( 16.666f, 0.0f, dy );
		}
	}

	// S-debug: continuous mouse-look spin through the rig's real input
	// path (HandleMouseLook every frame). VORTX_MOUSE_SPIN=<deg/frame>
	// (sign = direction). Combine with VORTX_SHOT_EVERY to catch
	// transient black-sky frames in captures.
	if( const char* spinEnv = std::getenv( "VORTX_MOUSE_SPIN" ) )
		if( m_playerRig )
		{
			const float deg = (float)atof( spinEnv );
			const float dx = -deg / ( 0.05f * 16.666f ); // sens*delta(ms)
			const char* spinPEnv = std::getenv( "VORTX_MOUSE_SPIN_PITCH" );
			const float degP = spinPEnv ? (float)atof( spinPEnv ) : 0.0f;
			const float dy = -degP / ( 0.05f * 16.666f );
			m_playerRig->HandleMouseLook( 16.666f, dx, dy );
		}

	// Terrain-demo debug: VORTX_RIG_TRACE=1 prints the rig camera Y
	// every 30 frames (proves the rig rests on the heightfield).
	if( std::getenv( "VORTX_RIG_TRACE" ) && m_playerRig )
	{
		static int s_traceFrame = 0;
		if( ++s_traceFrame % 30 == 0 )
		{
			btRigidBody* b = ( m_playerRig->getBulletObject() )
							 ? m_playerRig->getBulletObject()->getBody() : nullptr;
			if( b )
			{
				const btVector3 o = b->getWorldTransform().getOrigin();
				cout << "RIG_TRACE frame " << s_traceFrame << " body ("
					 << o.x() << "," << o.y() << "," << o.z() << ")" << endl;
			}
		}
	}

	// S-debug: hold a movement key through the rig's real input path.
	// VORTX_WALK=w|a|s|d — combine with VORTX_MOUSE_SPIN for
	// walk-while-turning repros.
	if( const char* walkEnv = std::getenv( "VORTX_WALK" ) )
		if( m_playerRig )
		{
			const char k = walkEnv[0];
			m_playerRig->HandleKeyboardInput(
				k == 'w', k == 's', k == 'a', k == 'd',
				false, false, false );
		}

	// S-debug: standalone debug camera, fully independent of the player
	// rig (the rig owns and updates its own camera every frame — never
	// steer it from here). VORTX_DEBUG_CAM=1 creates + activates it;
	// VORTX_CAM_YAW/PITCH (degrees) and VORTX_CAM_POS="x,y,z" steer it.
	static lsCamera* s_debugCam = nullptr;
	if( std::getenv( "VORTX_DEBUG_CAM" ) && m_viewer_scene && ! s_debugCam )
	{
		s_debugCam = new lsCamera( m_viewer_scene );
		if( s_debugCam )
		{
			s_debugCam->setName( "DebugCam" );
			s_debugCam->Set_Perspective( 45.0, 1820.0 / 1080.0, 0.1, 1000.0 );
			s_debugCam->Set_Pos( 0, 5, 8 );	// rig start pose
			m_viewer_scene->Add_Node( s_debugCam );
			m_viewer_scene->setActiveCamera( s_debugCam );
			cout << "DemoScene: debug camera active" << endl;
		}
	}
	if( s_debugCam && m_viewer_scene
	    && m_viewer_scene->getActiveCamera() == s_debugCam )
	{
		const char* yawEnv = std::getenv( "VORTX_CAM_YAW" );
		const char* pitchEnv = std::getenv( "VORTX_CAM_PITCH" );
		const char* posEnv = std::getenv( "VORTX_CAM_POS" );
		if( yawEnv || pitchEnv )
			s_debugCam->Set_Rot(
				pitchEnv ? (float)atof( pitchEnv ) : 0.0f,
				yawEnv ? (float)atof( yawEnv ) : 0.0f, 0.0f );
		if( posEnv )
		{
			float px = 0, py = 5, pz = 8;
			sscanf( posEnv, "%f,%f,%f", &px, &py, &pz );
			s_debugCam->Set_Pos( px, py, pz );
		}
	}
#endif

	// lsFlyCam survey camera: F6 toggles it live (noclip-style); the
	// env vars force it ON once at startup for scripted captures.
	// Pose: VORTX_CAM_POS / VORTX_CAM_YAW/PITCH; speed: VORTX_FLYSPEED.
	// Scripted flyover:
	//   VORTX_FLYPATH=orbit:cx,cy,cz,radius,periodSec[,pitchDeg]
	//   VORTX_FLYPATH=tour:x0,y0,z0;x1,y1,z1;...  (loops at flySpeed)
	{
		static bool s_flyEnvChecked = false;
		if( ! s_flyEnvChecked )
		{
			s_flyEnvChecked = true;
			if( std::getenv( "VORTX_FLYCAM" ) && ! m_flyCamActive )
			{
				toggleFlyCam();
				float px = kFlyDefX, py = kFlyDefY, pz = kFlyDefZ;
				float yaw = 0.0f, pitch = -25.0f;
				if( const char* e = std::getenv( "VORTX_CAM_POS" ) )
					sscanf( e, "%f,%f,%f", &px, &py, &pz );
				if( const char* e = std::getenv( "VORTX_CAM_YAW" ) )
					yaw = ( float )atof( e );
				if( const char* e = std::getenv( "VORTX_CAM_PITCH" ) )
					pitch = ( float )atof( e );
				m_flyCam->Set_Pos( px, py, pz );
				m_flyCam->setYawPitch( yaw, pitch );

				if( const char* path = std::getenv( "VORTX_FLYPATH" ) )
				{
					if( strncmp( path, "orbit:", 6 ) == 0 )
					{
						float cx = 0, cy = 100, cz = 0, r = 300,
							  per = 60, pp = -35;
						sscanf( path + 6, "%f,%f,%f,%f,%f,%f",
								&cx, &cy, &cz, &r, &per, &pp );
						m_flyCam->Start_Orbit( lsVector3( cx, cy, cz ),
											   r, per, pp, false );
						cout << "DemoScene: flycam orbit c(" << cx << ","
							 << cy << "," << cz << ") r=" << r
							 << " period=" << per << "s" << endl;
					}
					else if( strncmp( path, "tour:", 5 ) == 0 )
					{
						std::string wps( path + 5 );
						size_t start = 0;
						while( start < wps.size() )
						{
							size_t end = wps.find( ';', start );
							std::string tok = wps.substr( start,
								end == std::string::npos ? end
														 : end - start );
							float wx, wy, wz;
							if( sscanf( tok.c_str(), "%f,%f,%f",
										&wx, &wy, &wz ) == 3 )
								m_flyCam->Add_Waypoint(
									lsVector3( wx, wy, wz ) );
							if( end == std::string::npos ) break;
							start = end + 1;
						}
						m_flyCam->Start_Waypoints( -35.0f );
						cout << "DemoScene: flycam waypoint tour" << endl;
					}
				}
				else
				{
					cout << "DemoScene: flycam active at (" << px << ","
						 << py << "," << pz << ")" << endl;
				}
			}
		}
	}

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

void DemoScene::toggleFlyCam()
{
	if( ! m_viewer_scene ) return;
	if( ! m_flyCam )
	{
		m_flyCam = new lsFlyCam( m_viewer_scene );
		m_flyCam->getCamera()->Set_Perspective( 45.0, 1820.0 / 1080.0, 0.1, 1000.0 );
		m_viewer_scene->Add_Node( m_flyCam );
		if( const char* e = std::getenv( "VORTX_FLYSPEED" ) )
			m_flyCam->setFlySpeed( ( float )atof( e ) );
	}
	if( ! m_flyCamActive )
	{
		// start from the current camera's position (mouse to look,
		// WASD to fly, Space/Ctrl up/down, Shift sprint)
		lsCamera* cur = m_viewer_scene->getActiveCamera();
		if( cur && cur != m_flyCam->getCamera() )
		{
			m_flyCam->Set_Pos( cur->ModelMatrix.matrix[12],
							   cur->ModelMatrix.matrix[13],
							   cur->ModelMatrix.matrix[14] );
			// sync orientation: extract yaw/pitch from the current
			// camera's forward (-Z column of the ModelMatrix)
			const float fx = -cur->ModelMatrix.matrix[2],
						fy = -cur->ModelMatrix.matrix[6],
						fz = -cur->ModelMatrix.matrix[10];
			const float yawDeg = std::atan2( -fx, -fz ) * 57.29578f;
			const float pitchDeg = std::asin( std::max( -1.0f,
						std::min( 1.0f, fy ) ) ) * 57.29578f;
			m_flyCam->setYawPitch( yawDeg, pitchDeg );
		}
		m_viewer_scene->setActiveCamera( m_flyCam->getCamera() );
		// sun shadow map must follow the ACTIVE camera, not the player
		if( m_sun_light )
			m_sun_light->setFollowCamera( m_flyCam->getCamera() );
		m_flyCamActive = true;
		cout << "DemoScene: flycam ON (F6 returns to the player rig)"
			 << endl;
	}
	else
	{
		if( m_playerRig && m_playerRig->getCamera() )
		{
			m_viewer_scene->setActiveCamera( m_playerRig->getCamera() );
			if( m_sun_light )
				m_sun_light->setFollowCamera( m_playerRig->getCamera() );
		}
		m_flyCamActive = false;
		cout << "DemoScene: flycam OFF - player rig active" << endl;
	}
}

void DemoScene::movePlayer(bool f, bool b, bool l, bool r, bool jump, bool sprint, bool crouch)
{
	if (m_flyCamActive && m_flyCam)
	{
		// flycam owns movement while toggled on: jump=up, crouch=down
		m_flyCam->HandleFlyInput(f, b, l, r, jump, crouch, sprint);
		return;
	}
	if (m_playerRig)
	{
		m_playerRig->HandleKeyboardInput(f, b, l, r, crouch, jump, sprint);
	}
}

void DemoScene::rotatePlayerView( double delta_x, double delta_y)
{
	if (m_flyCamActive && m_flyCam) {
		m_flyCam->HandleFlyLook(static_cast<float>(delta_x), static_cast<float>(delta_y));
		return;
	}
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

