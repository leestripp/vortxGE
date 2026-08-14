/**
 * @file lsCamera.h
 * @brief Camera class for scene viewing and projection
 * 
 * lsCamera represents a viewpoint into the 3D scene.
 * It handles:
 * - View matrix (camera transformation)
 * - Projection matrix (perspective or orthographic)
 * - Frustum for view culling
 * 
 * @section camera_modes Camera Modes
 * 
 * - LSCM_PERSPECTIVE: Standard perspective projection
 * - LSCM_ORTHOGRAPHIC: Parallel projection (no depth)
 * 
 * @section usage Usage
 * @code
 * lsCamera* cam = new lsCamera(scene);
 * cam->Set_Perspective(60.0f, 16.0f/9.0f, 0.1f, 1000.0f);
 * cam->Set_Pos(0, 5, 10);
 * cam->Set_Rot(-20, 0, 0);
 * @endcode
 * 
 * @see lsFirstPersonRig for first-person controls
 * @see lsFrustum for view frustum
 */

#pragma once

// math
#include "../math/lsFrustum.h"

// scene
#include "../scene/lsAnimNode.h"

//=========================================================================
// Camera Mode Constants
//=========================================================================

/** @brief Perspective projection (3D with depth) */
const int LSCM_PERSPECTIVE      = 0;
/** @brief Orthographic projection (2D/technical) */
const int LSCM_ORTHOGRAPHIC     = 1;

/**
 * @class lsCamera
 * @brief Camera viewpoint for scene rendering
 * 
 * lsCamera provides the viewpoint for rendering the scene.
 * It maintains:
 * - View matrix (camera's position and orientation)
 * - Projection matrix (how 3D is projected to 2D)
 * - View frustum (for culling)
 * 
 * @section projection Projection
 * 
 * Perspective projection (default):
 * - Objects get smaller with distance
 * - FOV determines field of view angle
 * - Near/far planes clip distant objects
 * 
 * Orthographic projection:
 * - No perspective (parallel lines stay parallel)
 * - Used for technical/2D rendering
 */
class lsCamera : public lsAnimNode
{
public:
    /**
     * @brief Construct a camera
     * @param parent Parent node (usually lsScene)
     */
	lsCamera( lsNode* parent );
	
    /**
     * @brief Destructor
     */
	virtual ~lsCamera();

    //=========================================================================
	// Virtual Overrides
	//=========================================================================
	
    /**
     * @brief Update camera state
     * 
     * Updates view matrix based on position/rotation.
     * 
     * @param scn Scene
     * @param render Renderer
     * @param delta Time since last frame
     */
	virtual void Move( lsScene* scn, lsRender* render, double delta );
	
    /**
     * @brief Optimize camera
     * 
     * Updates frustum for culling.
     * 
     * @param cam Active camera (usually this)
     * @param pass Current render pass
     * @param scn Scene
     * @param render Renderer
     */
	virtual void Optimize( lsCamera* cam, int pass_type, lsScene* scn, lsRender* render );
	
    //=========================================================================
	// Projection Setup
	//=========================================================================
	
    /**
     * @brief Setup perspective projection
     * 
     * @param view_angle Vertical FOV in degrees
     * @param width_ratio Aspect ratio (width/height)
     * @param nplane Near clipping plane
     * @param fplane Far clipping plane
     */
	void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
	
    /**
     * @brief Set aspect ratio
     * @param vr width/height ratio
     */
	void Set_ViewRatio( float vr );
	
    /**
     * @brief Setup orthographic projection
     * 
     * @param xMin Left bound
     * @param xMax Right bound
     * @param yMin Bottom bound
     * @param yMax Top bound
     * @param zMin Near bound
     * @param zMax Far bound
     */
	void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
	
    /**
     * @brief Setup cubic orthographic projection
     * 
     * @param cube Half-size of cube (extends -cube to +cube)
     */
	void Set_Orthographic( float cube );
	
    /**
     * @brief Update camera matrices
     * 
     * Recalculates view and projection matrices.
     * Usually called after changing position/rotation.
     */
	void Update_Camera();
	
    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Serialize camera
     * @return String representation
     */
	virtual const string& toString();

    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get camera mode
     * @return LSCM_PERSPECTIVE or LSCM_ORTHOGRAPHIC
     */
	// Projection parameters (L2 CSM needs them for cascade fitting)
	float getCam_fov() const { return cam_fov; }
	float getCam_ratio() const { return cam_ratio; }
	float getCam_near() const { return cam_near; }
	float getCam_far() const { return cam_far; }

	int getCamera_mode() const
	{
		return camera_mode;
	}

    /**
     * @brief Set camera mode
     * @param val LSCM_* constant
     */
	void setCamera_mode( int val )
	{
		camera_mode = val;
	}

    //=========================================================================
    // Public Data Members
    //=========================================================================
	
    /** @brief View matrix (world to camera space) */
	lsMatrix4x4 ViewMatrix;
	
    /** @brief View frustum for culling */
	lsFrustum frustum;
	
	//=========================================================================
	// Perspective Parameters
	//=========================================================================
	
    /** @brief Field of view (degrees) */
	float cam_fov;
    
    /** @brief Aspect ratio (width/height) */
	float cam_ratio;
    
    /** @brief Near clipping plane */
	float cam_near;
    
    /** @brief Far clipping plane */
	float cam_far;
	
	//=========================================================================
	// Orthographic Parameters
	//=========================================================================
	
    /** @brief Left bound */
	float orth_left;
    
    /** @brief Right bound */
	float orth_right;
    
    /** @brief Top bound */
	float orth_top;
    
    /** @brief Bottom bound */
	float orth_bottom;
	
	//=========================================================================
	// Auto Update
	//=========================================================================
	
    /** @brief Frames between frustum updates (0=every frame) */
	int update_interval;
    
    /** @brief Counter for update interval */
	int update_counter;
	
private:
    /** @brief Camera mode (perspective/orthographic) */
	int camera_mode;
};
