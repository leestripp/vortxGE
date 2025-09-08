// Threads
#include <mutex>

// core
#include "../vortx_debug.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector3.h"
#include "../properties/lsProperty_Float.h"

// animation
#include "../animation/lsAnimationManager.h"

#include "lsCamera.h"


lsCamera::lsCamera( lsNode* parent ) : lsAnimNode(parent)
{
    setNode_type( LSNT_CAMERA );
    setName("Camera");

    // Defaults
    cam_fov = 45.0;
    cam_ratio = 1.0;
    cam_near = 1.0;
    cam_far = 1000.0;
    cam_orthoCube = 1.0;
    update_interval = 50;
    update_counter = 0;

    ViewMatrix.LoadIdentity();
    Set_Perspective( cam_fov, cam_ratio, cam_near, cam_far );

    // Add default properties
    lsProperty_Vector3 *pv3;
    lsProperty_Float *pv1;

    pv3 = new lsProperty_Vector3( &position );
    pv3->setProp_name( "Position" );
    pv3->setProp_id( LSPROPID_POSITION );
    Add_Property( pv3 );

    pv3 = new lsProperty_Vector3( &rotation );
    pv3->setProp_name( "Rotation" );
    pv3->setProp_id( LSPROPID_ROTATION );
    Add_Property( pv3 );

    pv3 = new lsProperty_Vector3( &scale );
    pv3->setProp_name( "Scale" );
    pv3->setProp_id( LSPROPID_SCALE );
    Add_Property( pv3 );

    // Camera settings
    pv1 = new lsProperty_Float( &cam_fov );
    pv1->setProp_name( "Camera FOV" );
    pv1->setProp_id( LSPROPID_CAMERA_FOV );
    Add_Property( pv1 );

    pv1 = new lsProperty_Float( &cam_ratio );
    pv1->setProp_name( "Camera View Ratio" );
    pv1->setProp_id( LSPROPID_CAMERA_RATIO );
    Add_Property( pv1 );

    pv1 = new lsProperty_Float( &cam_near );
    pv1->setProp_name( "Camera Near Plane" );
    pv1->setProp_id( LSPROPID_CAMERA_NEAR );
    Add_Property( pv1 );

    pv1 = new lsProperty_Float( &cam_far );
    pv1->setProp_name( "Camera Far Plane" );
    pv1->setProp_id( LSPROPID_CAMERA_FAR );
    Add_Property( pv1 );

    pv1 = new lsProperty_Float( &cam_orthoCube );
    pv1->setProp_name( "Camera Orthographic Cube" );
    pv1->setProp_id( LSPROPID_CAMERA_ORTHOCUBE );
    Add_Property( pv1 );
}

lsCamera::~lsCamera()
{
}

void lsCamera::Move( __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsOpenGL* opengl, __attribute__((unused)) double delta )
{
    // Auto update
    update_counter++;
    if( update_counter > update_interval )
    {
        update_counter = 0;
        switch( camera_mode )
        {
        case LSCM_PERSPECTIVE:
            Set_Perspective( cam_fov, cam_ratio, cam_near, cam_far );
            break;

        case LSCM_ORTHOGRAPHIC:
            Set_Orthographic( cam_orthoCube );
            break;
        }
    }

    // Set transform
    lsMatrix4x4 transmat;
    lsMatrix4x4 rotmat;
    lsMatrix4x4 scalemat;

    transmat.TranslateMatrix( position.x, position.y, position.z );
    rotmat.RotateMatrix( rotation.x, 1, 0, 0 );
    rotmat.RotateMatrix( rotation.y, 0, 1, 0 );
    rotmat.RotateMatrix( rotation.z, 0, 0, 1 );
    scalemat.ScaleMatrix( scale.x, scale.y, scale.z );

    if( getParent() )
        ModelMatrix = getParent()->ModelMatrix;
    else
        ModelMatrix.LoadIdentity();
	
    // Build matrix
     ModelMatrix = ModelMatrix *  transmat * rotmat * scalemat;
	
#ifdef VORTX_DEBUG_NODES
    // debug
    ModelMatrix.debug( "lsCamera ModelMatrix: " + getName() );
#endif

    // Camera and Light ViewMatrix updates
    Update_Camera();
}

void lsCamera::Optimize( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) lsPass* pass, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsOpenGL* opengl )
{
    // Do nothing
}

// Camera settings

void lsCamera::Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane )
{
    cam_fov = view_angle;
    cam_ratio = width_ratio;
    cam_near = nplane;
    cam_far = fplane;
	
    frustum.Set_Perspective( view_angle, width_ratio, nplane, fplane );
    camera_mode = LSCM_PERSPECTIVE;
}

void lsCamera::Set_ViewRatio( float vr )
{
    cam_ratio = vr;

    frustum.Set_ViewRatio( vr );
    camera_mode = LSCM_PERSPECTIVE;
}

void lsCamera::Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
    frustum.Set_Orthographic( xMin, xMax, yMin, yMax, zMin, zMax );
    camera_mode = LSCM_ORTHOGRAPHIC;
}

void lsCamera::Set_Orthographic( float cube )
{
    cam_orthoCube = cube;

    frustum.Set_Orthographic( -cube, cube, -cube, cube, -cube, cube );
    camera_mode = LSCM_ORTHOGRAPHIC;
}

/**
 Update both Camera and Light ViewMatrix data
 */
void lsCamera::Update_Camera()
{
    lsVector3 eye, lookat, vup;

    eye = lsVector3( ModelMatrix.matrix[12], ModelMatrix.matrix[13], ModelMatrix.matrix[14] );
    lookat = lsVector3( ModelMatrix.matrix[12]+ModelMatrix.matrix[8], ModelMatrix.matrix[13]+ModelMatrix.matrix[9], ModelMatrix.matrix[14]+ModelMatrix.matrix[10] );
    vup = lsVector3( ModelMatrix.matrix[4], ModelMatrix.matrix[5], ModelMatrix.matrix[6] );

    ViewMatrix.LoadIdentity();
    ViewMatrix.LookAt( eye, lookat, vup );

#ifdef VORTX_DEBUG_NODES
    // debug : use stringstream for threads.
    stringstream msg;
    msg << "View from : " << getName() << "  Mode: ";
    if( camera_mode == LSCM_PERSPECTIVE )
    {
        msg << "Perspective" << endl;
    } else msg << "Orthographic" << endl;
    msg << "lsCamera ViewMatrix: ";
    ViewMatrix.debug( msg.str() );
#endif

    /* Frustum */
    frustum.Build_CullPlanes( eye, lookat, vup );
}


// Save data
const string& lsCamera::toString()
{
    save_str = "add_camera ";
    if( getName() != "" )
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "`Camera`";
    }

    if( getParent() )
    {
        save_str += " `";
        save_str += getParent()->getName();
        save_str += "`";
    } else
    {
        save_str += " `NONE`";
    }

    // Pos, Rot and scale
    save_str += " ";
    save_str += position.toString();
    save_str += " ";
    save_str += rotation.toString();
    save_str += " ";
    save_str += scale.toString();

    return save_str;
}

