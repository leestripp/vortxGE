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
	
	// Quad controls.
    lsScene *m_quad_scene;
    lsViewer *m_quad_viewer;
	
    // Main Scene controls
    lsScene *m_viewer_scene;
    lsViewer *m_viewer;
	
    // Player Rig
    lsFirstPersonRig* m_playerRig; // Changed type to lsFirstPersonRig
    
    // Audio Speaker
    lsSpeaker* m_speaker; // Add a speaker object to the scene
};
