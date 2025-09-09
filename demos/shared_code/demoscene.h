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
class lsLight_Spot;
class lsNode_FirstPersonRig; // Forward declare our new class

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
    // vortxGE - Scene events and Camera
    void Update( double delta );
    void Render();

protected:
    void setupControls();
    void setupTestScene();
    void setupObjectEmitter();
    // Scene Events
    void processEvent( lsEvent* event );

private:
    vortxGE     *vortxge;

    // Globals
    lsScene *viewer_scene;
    lsViewer *viewer;
    // lights
    lsLight_Spot *spot_light;

    // Player Rig
    lsNode_FirstPersonRig* m_playerRig;
};
