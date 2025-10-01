#pragma once

// opengl
#include "../render/lsOpenGL.h"

#include "lsData.h"

class btCollisionShape;
class lsMesh;


class lsBulletShape : public lsData
{
public:
    lsBulletShape( lsOpenGL *opengl );
    virtual ~lsBulletShape();
    // opengl
    lsOpenGL *ogl;

	// build shapes
	void Make_Plane( float x, float y, float z );
	void Make_Box( float x, float y, float z );
	void Make_Sphere( float r );
	void Make_Pill( float radius, float height );
	// From vbm file via lsBatch_Bullet
	void Make_ConvexHull( const string& filename, lsOpenGL *opengl );
    void Make_TriMesh( const string& filename, float size = 1.0 );
	// From lsMesh
	void Make_ConvexHull_FromMesh( lsMesh* mesh );
    void Make_TriMesh_FromMesh( lsMesh* mesh );
	
	btCollisionShape *getShape();
	
private:
    btCollisionShape *shape;
};
