#ifndef DEMOSCENE_H
#define DEMOSCENE_H

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

using namespace std;

class DemoScene
{
public:
    DemoScene();
    virtual ~DemoScene();

    bool Init();
    void Resize( int x, int y, int w, int h );
    // Camera movement
    void moveCamera( bool f, bool b, bool l, bool r, double delta, double factor );
    void rotateCamera( double rot_x, double rot_y, double rot_z );
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
    // Camera
    lsCamera *camera;
    lsObject *camera_base;
    // movement
    lsVector3 cam_pos;
    lsVector3 move_walk;
    lsVector3 move_strafe;
};

#endif // DEMOSCENE_H
