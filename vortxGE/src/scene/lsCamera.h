#pragma once

// math
#include "../math/lsFrustum.h"

// scene
#include "../scene/lsAnimNode.h"

const int LSCM_PERSPECTIVE      = 0;
const int LSCM_ORTHOGRAPHIC     = 1;

//! A scene camera
/*!
 * Used to view 3D objects. Yep!
 *
 */
class lsCamera : public lsAnimNode
{
public:
	lsCamera( lsNode* parent );
	virtual ~lsCamera();

	// from lsNode
	virtual void Move( lsScene* scn, lsRender* render, double delta );
	virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
	
	void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
	void Set_ViewRatio( float vr );
	void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
	void Set_Orthographic( float cube );
	void Update_Camera();
	
	// Save data
	virtual const string& toString();

	int getCamera_mode() const
	{
		return camera_mode;
	}

	void setCamera_mode( int val )
	{
		camera_mode = val;
	}

	// Public data
	lsMatrix4x4 ViewMatrix;
	lsFrustum frustum;
	// perspective.
	float cam_fov;
	float cam_ratio;
	float cam_near;
	float cam_far;
	// Orth params
	float orth_left;
	float orth_right;
	float orth_top;
	float orth_bottom;
	
	// Auto update
	int update_interval;
	int update_counter;
	
private:
	int camera_mode;
};
