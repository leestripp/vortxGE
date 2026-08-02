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

class vortxGE;
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
    // Player movement
    void movePlayer(bool f, bool b, bool l, bool r, bool jump, bool sprint, bool crouch);
    void rotatePlayerView( double delta_x, double delta_y);
    void handleMouseButtons(bool left, bool right); // Add this method
    // vortxGE - Scene events and Camera
    void Update( double delta );
    void Render();

protected:
    void setupControls();
    void setupTestScene();
    void setupShadows();
    void setupObjectEmitter();
    // Scene Events
    void processEvent( lsEvent* event );
	
private:
    vortxGE     *m_vortxge;

    // Framebuffer pixel dimensions (set by Init() / Resize()).
    int m_view_width;
    int m_view_height;
	
	// Quad controls.
    lsScene *m_quad_scene;
    lsViewer *m_quad_viewer;
	
    // Main Scene controls
    lsScene *m_viewer_scene;
    lsViewer *m_viewer;
	
    // Player Rig
    lsFirstPersonRig* m_playerRig = nullptr; // Changed type to lsFirstPersonRig
    lsLight_Direct* m_sun_light = nullptr;
    
    // Audio Speaker
    lsSpeaker* m_speaker = nullptr; // Add a speaker object to the scene
};
