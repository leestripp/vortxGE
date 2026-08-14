#include "lsLight_Direct.h"

// math
#include "../math/lsMatrix4x4.h"
#include "../math/lsVector3.h"

// animation
#include "../animation/lsAnimationManager.h"

lsLight_Direct::lsLight_Direct(lsNode* parent) : lsLight(parent)
{
    setLight_type(LSLT_DIRECT);
    setName("Direct Light");
    
    // Set more sensible defaults.
    // void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
    Set_Orthographic(-100.0, 100.0, -100.0, 100.0, 10.0, 1000.0);
    
    // Direct lights have no attenuation, so set this low.
    intensity = 5.0f;
    
    // Initialize follow camera to NULL
    m_followCamera = nullptr;
    // Light direction
    m_lightDirection = lsVector3(0.0, 1.0, 0.0);
}

lsLight_Direct::~lsLight_Direct()
{
}

void lsLight_Direct::updateFollowCamera()
{
	if( !m_followCamera )
		return;

	// The sun follows the camera so its ortho shadow box stays with the
	// player. We snap to whole-texel boundaries in buildShadowmap_matrix,
	// so sub-texel camera motion does not swim the shadows.
	lsVector3 cam_pos = lsVector3(
		m_followCamera->ModelMatrix.matrix[12],
		m_followCamera->ModelMatrix.matrix[13],
		m_followCamera->ModelMatrix.matrix[14] );
	lsVector3 final_pos = cam_pos + position;

	ModelMatrix.matrix[12] = final_pos.x;
	ModelMatrix.matrix[13] = final_pos.y;
	ModelMatrix.matrix[14] = final_pos.z;

	// Light direction follows camera offset.
	m_lightDirection = position;
}

void lsLight_Direct::Move( __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render, __attribute__((unused)) double delta )
{
	// Auto update
	update_counter++;
	if( update_counter > update_interval )
	{
		update_counter = 0;
		switch( getCamera_mode() )
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
	
	updateFollowCamera();
	
#ifdef VORTX_DEBUG_NODES
	// debug
	ModelMatrix.debug( "lsCamera ModelMatrix: " + getName() );
#endif
	
	// Camera and Light ViewMatrix updates
	Update_Camera();
}

const string& lsLight_Direct::toString()
{
    save_str = "add_light direct ";
    if( getName() != "" )
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "`Direct`";
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
