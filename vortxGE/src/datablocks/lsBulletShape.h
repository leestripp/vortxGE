#pragma once

// render
#include "../render/lsRender.h"

#include "lsData.h"

#include <vector>

class btCollisionShape;
class lsMesh;


class lsBulletShape : public lsData
{
public:
    lsBulletShape( lsRender *render );
    virtual ~lsBulletShape();
    // render
    lsRender *m_render;

	// build shapes
	void Make_Plane( float x, float y, float z );
	void Make_Box( float x, float y, float z );
	void Make_Sphere( float r );
	void Make_Pill( float radius, float height );
	// From vbm file via lsBatch_Bullet
	void Make_ConvexHull( const string& filename, lsRender *render );
    void Make_TriMesh( const string& filename, float size = 1.0 );
	// From lsMesh
	void Make_ConvexHull_FromMesh( lsMesh* mesh );
    void Make_TriMesh_FromMesh( lsMesh* mesh );
	// Heightfield terrain (btHeightfieldTerrainShape). A COPY of the
	// normalized [0,1] heights is kept so the shape stays valid.
	// The shape is centre-origin in ALL axes: X/Z span
	// +/- (n-1)*cell/2 and Y spans +/- heightScale/2 around the body
	// position — place the body at terrain_base_y + heightScale/2.
	void Make_Heightfield( int w, int h, const float* heights,
	                       float heightScale, float cellX, float cellZ );
	
	btCollisionShape *getShape();
	
private:
    btCollisionShape *shape;
    std::vector<float> m_heightfieldData; // owned copy for Make_Heightfield
};
