/**
 * @file demoscene.h
 * @brief Demo scene class declaration
 * 
 * This class manages the demo scene that showcases the vortxGE engine.
 * It provides a complete example of how to use the engine for:
 * - Scene setup and management
 * - Object creation and manipulation
 * - Camera and lighting
 * - Physics integration
 * - User input handling
 * - Animation
 * 
 * @see demoscene.cpp
 */

#pragma once

// math
#include <math/lsVector3.h>
#include <assets/lsStreamingManager.h>
#include <assets/lsAsyncFileLoader.h>

class vortxGE;

// Backend selection (RenderType) — no backend deps, safe to include.
#include "render/lsRenderTypes.h"
class lsBatch_Quad;
class lsScene;
class lsViewer;
class lsObject;
class lsCamera;
class lsEvent;
class lsFirstPersonRig;
class lsSpeaker;
class lsLight_Direct;

using namespace std;

class DemoScene
{
public:
    DemoScene();
    virtual ~DemoScene();

    bool Init();
    void Resize( int x, int y, int w, int h );

    /// Plan.md Part 3 Phase R2: explicit render pipeline (created when
    /// VORTX_PIPELINE=1 is set at Init). Owned by DemoScene.
    class lsRenderPipeline* m_pipeline = nullptr;
    void initPipeline();
    // Player movement
    void movePlayer(bool f, bool b, bool l, bool r, bool jump, bool sprint, bool crouch);
    void rotatePlayerView( double delta_x, double delta_y);
    void handleMouseButtons(bool left, bool right); // Add this method
    /**
     * @brief Set the renderer backend + native window BEFORE Init().
     *
     * The host app decides the backend (vortxGE::PreferredRendererType())
     * before creating its window, then hands both down here. GL-specific
     * setup in Init() (glewInit, GL state) is skipped for Vulkan.
     */
    void SetRendererPreference( RenderType type, void* nativeWindow )
    {
        m_prefRenderer = type;
        m_prefWindow = nativeWindow;
    }

    // vortxGE - Scene events and Camera
    void Update( double delta );
    void Render();

    /// Engine instance (valid after Init()) — e.g. for getRender()->Screenshot().
    vortxGE* getEngine() { return m_vortxge; }

protected:
    void setupControls();

    /// Selected scene name (VORTX_SCENE, default "demoscene")
    std::string m_sceneName = "demoscene";
    // Live-link (VORTX_LIVELINK=1): watch the scene .vgs for external
    // edits (Blender push) and re-apply transforms in place
    bool        m_liveLink = false;
    long        m_liveMtime = 0;
    double      m_liveCheckAcc = 0.0;
    void setupTestScene();
    void setupShadows();
    void setupObjectEmitter();
    void setupTestModel();      // debug-gated (VORTX_TEST_MODEL)
    void setupSampleModels();   // Khronos sample models (data/testmodels/)
    // Scene Events
    void processEvent( lsEvent* event );
	
private:
    vortxGE     *m_vortxge;

    RenderType  m_prefRenderer = RenderType::None;  // backend preference (None = engine auto)
    RenderType  m_activeRenderer = RenderType::None; // resolved backend in use
    void        *m_prefWindow = nullptr;            // native window for Vulkan surface

    // Framebuffer pixel dimensions (set by Init() / Resize()).
    int m_view_width;
    int m_view_height;
	
	// Quad controls.
    lsScene *m_quad_scene;
    lsViewer *m_quad_viewer;
	
    // Main Scene controls
    lsScene *m_viewer_scene;
    lsStreamingManager m_streamingManager;	///< M4: distance-driven mesh streaming
    lsAsyncFileLoader m_asyncLoader{ 2 };		///< M5: worker pool for async mesh loads
    lsViewer *m_viewer;
	
    // Player Rig
    lsFirstPersonRig* m_playerRig = nullptr; // Changed type to lsFirstPersonRig
    lsLight_Direct* m_sun_light = nullptr;
    
    // Audio Speaker
    lsSpeaker* m_speaker = nullptr; // Add a speaker object to the scene
};
