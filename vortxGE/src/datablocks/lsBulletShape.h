#ifndef vortxGE_lsBulletShape_h
#define vortxGE_lsBulletShape_h

// opengl
#include "../opengl/lsOpenGL.h"

#include "lsData.h"

class btCollisionShape;

/**
 Holds the basic Bullet Shape to be attached to a lsBulletObject used by lsObjects
 */
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
	// From vbm file via lsBatch_Buller
	void Make_ConvexHull( const string& filename, lsOpenGL *opengl );
    void Make_TriMesh( const string& filename, float size = 1.0 );

	btCollisionShape *getShape();
	
private:
    btCollisionShape *shape;

};


#endif
