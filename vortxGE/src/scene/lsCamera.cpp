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
	// Orth ad default cube.
	orth_left = -1.0;
	orth_right = 1.0;
	orth_bottom = -1.0;
	orth_top = 1.0;
	
	// Update the camera's Projection matrix, Frustum.
	update_interval = 30;
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
	markDirty();
}

lsCamera::~lsCamera()
{
}

void lsCamera::Move( __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render, __attribute__((unused)) double delta )
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
			Set_Orthographic(orth_left, orth_right, orth_bottom, orth_top, cam_near, cam_far);
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
	markDirty();
	Update_Camera();
}

void lsCamera::Optimize( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) int pass_type, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render )
{
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
	markDirty();
}

void lsCamera::Set_ViewRatio( float vr )
{
	cam_ratio = vr;

	frustum.Set_ViewRatio( vr );
	camera_mode = LSCM_PERSPECTIVE;
	markDirty();
}

void lsCamera::Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
	orth_left = xMin;
	orth_right = xMax;
	orth_bottom = yMin;
	orth_top = yMax;
	cam_near = zMin;
	cam_far = zMax;
	
	frustum.Set_Orthographic( xMin, xMax, yMin, yMax, zMin, zMax );
	camera_mode = LSCM_ORTHOGRAPHIC;
	markDirty();
}

// In half bounds.

void lsCamera::Set_Orthographic( float cube )
{
	orth_left = -cube;
	orth_right = cube;
	orth_bottom = -cube;
	orth_top = cube;
	cam_near = -cube;
	cam_far = cube;
	
	frustum.Set_Orthographic( orth_left, orth_right, orth_bottom, orth_top, cam_near, cam_far );
	camera_mode = LSCM_ORTHOGRAPHIC;
}

void lsCamera::rebuildModelFromTRS()
{
	// Same T*R*S composition as lsNode::Move(), parented if attached.
	lsMatrix4x4 transmat, rotmat, scalemat;
	transmat.TranslateMatrix( position.x, position.y, position.z );
	rotmat.RotateMatrix( rotation.x, 1, 0, 0 );
	rotmat.RotateMatrix( rotation.y, 0, 1, 0 );
	rotmat.RotateMatrix( rotation.z, 0, 0, 1 );
	scalemat.ScaleMatrix( scale.x, scale.y, scale.z );

	if( getParent() )
		ModelMatrix = getParent()->ModelMatrix;
	else
		ModelMatrix.LoadIdentity();
	ModelMatrix = ModelMatrix * transmat * rotmat * scalemat;
}

void lsCamera::Update_Camera()
{
	// C2 fix: always rebuild. The dirty-flag early-out left cull planes
	// stale on camera-move paths that don't mark dirty (regression:
	// objects/sky sphere culled while in view). The flag remains for
	// the per-frame shadow-matrix cache only.
	
	lsVector3 eye, lookat_target, up_vector;
	
	eye = lsVector3( ModelMatrix.matrix[12], ModelMatrix.matrix[13], ModelMatrix.matrix[14] );
	
	// The camera's local -Z axis (forward) transformed to world space.
	// ModelMatrix.matrix[8], ModelMatrix.matrix[9], ModelMatrix.matrix[10] is the local +Z axis in world space (3rd column, 0-indexed).
	// So, the forward vector is the negative of this.
	lsVector3 forward_world(-ModelMatrix.matrix[8], -ModelMatrix.matrix[9], -ModelMatrix.matrix[10]);
	forward_world.normalize();

	// The point the camera is looking at is its position plus its forward vector.
	lookat_target = eye + forward_world;

	// The camera's local +Y axis (up) transformed to world space.
	// ModelMatrix.matrix[4], ModelMatrix.matrix[5], ModelMatrix.matrix[6] is the local +Y axis in world space (2nd column, 0-indexed).
	up_vector = lsVector3(ModelMatrix.matrix[4], ModelMatrix.matrix[5], ModelMatrix.matrix[6]);
	up_vector.normalize();

	ViewMatrix.LoadIdentity();
	ViewMatrix.LookAt( eye, lookat_target, up_vector );

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
	frustum.Build_CullPlanes( eye, lookat_target, up_vector );

	// View + frustum are now current; consumers (shadow-matrix cache)
	// may trust the clean flag until the next mutation.
	m_matricesDirty = false;
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

