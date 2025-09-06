#include <iostream>
// debug
#include "../vortx_debug.h"

// Bullet
#include <bullet/btBulletDynamicsCommon.h>

// math
#include "../math/lsMath.h"

#include "lsBatch_Bullet.h"


lsBatch_Bullet::lsBatch_Bullet( lsOpenGL *opengl ) : lsBatch(opengl)
{
  setBatch_type( LSBT_BULLET );
  setName("Bullet Batch");

  pIndexes = NULL;
  pVerts = NULL;
}

lsBatch_Bullet::~lsBatch_Bullet()
{
	Cleanup();
}

void lsBatch_Bullet::Draw( lsCamera *cam, lsObject *object, lsPass *pass )
{
    // Not used but must exsist
    if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass );
}

void lsBatch_Bullet::Update( __attribute__((unused)) double delta )
{
    // Not used but must exsist
}

void lsBatch_Bullet::Begin( GLuint nMaxVerts )
{
	Cleanup();

	nMaxIndexes = nMaxVerts;
	nNumIndexes = 0;
	nNumVerts = 0;

	// Allocate
	pIndexes = new GLushort[ nMaxIndexes ];
	pVerts = new Vector3f[ nMaxIndexes ];
}

void lsBatch_Bullet::AddTriangle( Vector3f verts[3] )
{
	const float e = 0.00001; // How small a difference to equate

	// Search for match - triangle consists of three verts
	for( GLuint iVertex = 0; iVertex < 3; iVertex++ )
	{
		GLuint iMatch = 0;
		for( iMatch = 0; iMatch < nNumVerts; iMatch++ )
		{
			// If the vertex positions are the same
			if( CloseEnough( pVerts[iMatch][0], verts[iVertex][0], e ) &&
               CloseEnough( pVerts[iMatch][1], verts[iVertex][1], e ) &&
               CloseEnough( pVerts[iMatch][2], verts[iVertex][2], e ) )
			{
				// Then add the index only
				pIndexes[ nNumIndexes ] = iMatch;
				nNumIndexes++;
				break;
			}
		} // for iMatch

		// No match for this vertex, add to end of list
		if( iMatch == nNumVerts && nNumVerts < nMaxIndexes && nNumIndexes < nMaxIndexes )
		{
			memcpy( pVerts[nNumVerts], verts[iVertex], sizeof(Vector3f));

			pIndexes[nNumIndexes] = nNumVerts;
			nNumIndexes++;
			nNumVerts++;
		}
	} // for iVertex
}

void lsBatch_Bullet::Cleanup()
{
	// Cleanup
	if( pIndexes ) delete [] pIndexes;
	if( pVerts ) delete [] pVerts;
	pIndexes = NULL;
    pVerts = NULL;
}

void lsBatch_Bullet::Copy_GLdata()
{
	// Not used.
}

void lsBatch_Bullet::Clear_GLdata()
{
	// Not used.
}


//! Convex Hull
/*!
 * Uses points for simple convex hull collisions.
 *
 * */
btConvexHullShape* lsBatch_Bullet::End_ConvexHull()
{
	btVector3 point;
	btConvexHullShape* hull = new btConvexHullShape;

    for( unsigned int index=0; index<nNumVerts; index++ )
	{
		// Just add verts
		point = btVector3( pVerts[index][0], pVerts[index][1], pVerts[index][2] );
		hull->addPoint( point );
	}

	Cleanup();
	return hull;
}

//! Static Meshes
/*!
 * Used for static mesh collisions.
 *
 * */
btBvhTriangleMeshShape* lsBatch_Bullet::End_TriMesh( float size )
{
	// Add verticies
	btVector3 *gVertices = new btVector3[ nNumVerts ];
	int vertStride = sizeof(btVector3);
	// Copy
    for( unsigned int index=0; index<nNumVerts; index++ )
	{
		gVertices[index][0] = pVerts[index][0];
		gVertices[index][1] = pVerts[index][1];
		gVertices[index][2] = pVerts[index][2];
	}

	// Set indexes
	int *gIndices = new int[ nNumIndexes ];
	int indexStride = 3*sizeof(int);
	// Copy
    for( unsigned int index=0; index<nNumIndexes; index++ )
	{
		gIndices[index] = pIndexes[index];
	}

	// make TriMesh Array
	btTriangleIndexVertexArray *m_indexVertexArrays = new btTriangleIndexVertexArray( nNumIndexes/3, gIndices, indexStride, nNumVerts,(btScalar*) &gVertices[0].x(), vertStride );

	// Shape
	bool useQuantizedAabbCompression = true;
	btVector3 aabbMin(-size,-size,-size), aabbMax(size,size,size);

	btBvhTriangleMeshShape* meshshape = new btBvhTriangleMeshShape( m_indexVertexArrays, useQuantizedAabbCompression, aabbMin, aabbMax );

	Cleanup();
	return meshshape;
}


// Utils ###########################################################################################

bool lsBatch_Bullet::CloseEnough( const float fCandidate, const float fCompare, const float fEpsilon )
{
    lsMath math;
    return math.approxEqual( fCandidate, fCompare, fEpsilon );
}

