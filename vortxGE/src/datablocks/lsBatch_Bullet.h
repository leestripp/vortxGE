#pragma once

#include "lsBatch.h"

#include "../render/lsRender.h"

// mmath
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"


class btBvhTriangleMeshShape;
class btConvexHullShape;


//! Bullet Mesh
/*!
 * Used to create a Convex Hull or Concave Triangle Mesh for use with the bullet physics engine.
 *
 * */
class lsBatch_Bullet : public lsBatch
{
public:
	lsBatch_Bullet( lsRender *render );
	virtual ~lsBatch_Bullet();

    // lsBatch
    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

	void Begin( GLuint nMaxVerts );
	void AddPoint( Vector3f vert );
	void AddTriangle( Vector3f verts[3] );
	btBvhTriangleMeshShape*  End_TriMesh( float size );
	btConvexHullShape*  End_ConvexHull();
	void Cleanup();

    // openGL data - on video card : needed by the asset system
    void Copy_GLdata();
    void Clear_GLdata();
	
	// Utils
	bool CloseEnough( const float fCandidate, const float fCompare, const float fEpsilon );
	
private:
	GLuint nMaxIndexes;
	GLuint nNumIndexes;
	GLuint nNumVerts;
	
	GLushort *pIndexes;
	Vector3f *pVerts;
};
