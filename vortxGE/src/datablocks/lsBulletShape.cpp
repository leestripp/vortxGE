// bullet
#include <bullet/btBulletDynamicsCommon.h>

// opengl
#include "../opengl/lsOpenGL.h"

// filesystem
#include "../filesystem/lsFileIO.h"

// datablocks
#include "lsString.h"
#include "lsBatch_Bullet.h"
#include "lsMesh.h"
#include "lsBatch_Mesh.h"

#include "lsBulletShape.h"

lsBulletShape::lsBulletShape( lsOpenGL *opengl ) : lsData()
{
  ogl = opengl;
  shape = NULL;
}

lsBulletShape::~lsBulletShape()
{
    if( shape ) delete shape;
}

// ###########################
// build shapes

void lsBulletShape::Make_Plane( float x, float y, float z )
{
	shape = new btStaticPlaneShape( btVector3( x, y, z ), 0.0 );
	setMem_used( sizeof(btStaticPlaneShape) );
}

/**
 * @brief Creates a simple box with origin at the center.
 *
 * @param x if set to 1, box x direction will be 1m
 * @param y as above...
 * @param z as above...
 * @return void
 */

void lsBulletShape::Make_Box( float x, float y, float z )
{
	shape = new btBoxShape( btVector3( x, y, z ) );
	setMem_used( sizeof(btBoxShape) );
}

void lsBulletShape::Make_Sphere( float r )
{
	shape = new btSphereShape( btScalar( r ) );
	setMem_used( sizeof(btSphereShape) );
}

void lsBulletShape::Make_Pill( float radius, float height )
{
	shape = new btCapsuleShape( radius, height );
	setMem_used( sizeof(btCapsuleShape) );
}

// ###########################
// From file

void lsBulletShape::Make_ConvexHull( const string& filename, lsOpenGL *opengl )
{
	lsBatch_Bullet *batch = NULL;
	int i;
	Vector3f verts[3];

	vector<string> list;
	string line;
	lsString convert;

	lsFileIO file;
	if( file.open( filename, LSIO_LINE_READ ) )
	{
		while( file.readline()  )
		{
			line = file.getLine();
			if( line != "" )
			{
				list.clear();
                convert.SplitString( line, ' ', list );

				if( list[0] == "batch" )
				{
					if( list[1] == "end" )
					{
						shape = batch->End_ConvexHull();
						break;
					} else
					{
						batch = new lsBatch_Bullet(opengl);
						if( batch )
						{
							i = convert.toInt( list[1] );
							batch->Begin( i );

						}
					}

				} else if( list[0] == "verts" )
				{
					verts[0][0] = convert.toFloat( list[1] );
					verts[0][1] = convert.toFloat( list[2] );
					verts[0][2] = convert.toFloat( list[3] );

					verts[1][0] = convert.toFloat( list[4] );
					verts[1][1] = convert.toFloat( list[5] );
					verts[1][2] = convert.toFloat( list[6] );

					verts[2][0] = convert.toFloat( list[7] );
					verts[2][1] = convert.toFloat( list[8] );
					verts[2][2] = convert.toFloat( list[9] );

				} else if( list[0] == "add_element" )
				{
					batch->AddTriangle( verts );

				}

			} // if line != ""
		} // while readline
	} // if open

	setMem_used( sizeof(shape) );
	// clean up
	if( batch ) delete batch;
}

void lsBulletShape::Make_TriMesh( const string& filename, float size )
{
	lsBatch_Bullet *batch = NULL;
	int i;
    float radius = size;    // Default is 1.0
	Vector3f verts[3];

	vector<string> list;
	string line;
	lsString convert;

	lsFileIO file;
	if( file.open( filename, LSIO_LINE_READ ) )
	{
		while( file.readline()  )
		{
			line = file.getLine();
			if( line != "" )
			{
				list.clear();
                convert.SplitString( line, ' ', list );

				if( list[0] == "bsphere" )
				{
					radius = convert.toFloat( list[4] );

				} else if( list[0] == "batch" )
				{
					if( list[1] == "end" )
					{
						shape = batch->End_TriMesh( radius );
						break;
					} else
					{
						batch = new lsBatch_Bullet( ogl );
						if( batch )
						{
							i = convert.toInt( list[1] );
							batch->Begin( i );

						}
					}
				} else if( list[0] == "verts" )
				{
					verts[0][0] = convert.toFloat( list[1] );
					verts[0][1] = convert.toFloat( list[2] );
					verts[0][2] = convert.toFloat( list[3] );

					verts[1][0] = convert.toFloat( list[4] );
					verts[1][1] = convert.toFloat( list[5] );
					verts[1][2] = convert.toFloat( list[6] );

					verts[2][0] = convert.toFloat( list[7] );
					verts[2][1] = convert.toFloat( list[8] );
					verts[2][2] = convert.toFloat( list[9] );

				} else if( list[0] == "add_element" )
				{
					batch->AddTriangle( verts );

				}

			} // if line != ""
		} // while readline
	} // if open

	setMem_used( sizeof(shape) );
	// clean up
	if( batch ) delete batch;
}

void lsBulletShape::Make_ConvexHull_FromMesh( lsMesh* mesh )
{
    if (!mesh) return;

    lsBatch_Bullet* batch = new lsBatch_Bullet(ogl);
    if (!batch) return;

    // Estimate total vertices to pre-allocate
    int totalVerts = 0;
    lsBatch* currentBatch = mesh->getBatch_list();
    while (currentBatch) {
        if (currentBatch->getBatch_type() == LSBT_MESH) {
            totalVerts += static_cast<lsBatch_Mesh*>(currentBatch)->getVertCount();
        }
        currentBatch = currentBatch->getNext_batch();
    }
    batch->Begin(totalVerts);

    // Add vertices from all batches
    currentBatch = mesh->getBatch_list();
    while (currentBatch) {
        if (currentBatch->getBatch_type() == LSBT_MESH) {
            lsBatch_Mesh* meshBatch = static_cast<lsBatch_Mesh*>(currentBatch);
            Vector3f* verts = meshBatch->getVerts();
            int vertCount = meshBatch->getVertCount();
            for (int i = 0; i < vertCount; ++i) {
                batch->AddPoint(verts[i]);
            }
        }
        currentBatch = currentBatch->getNext_batch();
    }

    shape = batch->End_ConvexHull();
    setMem_used(sizeof(btConvexHullShape)); // Approximation
    delete batch;
}

void lsBulletShape::Make_TriMesh_FromMesh( lsMesh* mesh )
{
    if (!mesh) return;

    lsBatch_Bullet* batch = new lsBatch_Bullet(ogl);
    if (!batch) return;

    // Estimate total vertices and indices
    int totalVerts = 0;
    int totalIndices = 0;
    lsBatch* currentBatch = mesh->getBatch_list();
    while (currentBatch) {
        if (currentBatch->getBatch_type() == LSBT_MESH) {
            lsBatch_Mesh* meshBatch = static_cast<lsBatch_Mesh*>(currentBatch);
            totalVerts += meshBatch->getVertCount();
            totalIndices += meshBatch->getIndexCount();
        }
        currentBatch = currentBatch->getNext_batch();
    }
    batch->Begin(totalIndices); // Begin expects max vertices, but for trimesh it's more about indices

    // Add triangles from all batches
    currentBatch = mesh->getBatch_list();
    while (currentBatch) {
        if (currentBatch->getBatch_type() == LSBT_MESH) {
            lsBatch_Mesh* meshBatch = static_cast<lsBatch_Mesh*>(currentBatch);
            Vector3f* verts = meshBatch->getVerts();
            GLushort* indices = meshBatch->getIndices();
            int indexCount = meshBatch->getIndexCount();

            for (int i = 0; i < indexCount; i += 3) {
                Vector3f triangle[3];
                triangle[0][0] = verts[indices[i]][0];
                triangle[0][1] = verts[indices[i]][1];
                triangle[0][2] = verts[indices[i]][2];

                triangle[1][0] = verts[indices[i+1]][0];
                triangle[1][1] = verts[indices[i+1]][1];
                triangle[1][2] = verts[indices[i+1]][2];

                triangle[2][0] = verts[indices[i+2]][0];
                triangle[2][1] = verts[indices[i+2]][1];
                triangle[2][2] = verts[indices[i+2]][2];
                batch->AddTriangle(triangle);
            }
        }
        currentBatch = currentBatch->getNext_batch();
    }

    shape = batch->End_TriMesh(mesh->getBSphere().radius);
    setMem_used(sizeof(btBvhTriangleMeshShape)); // Approximation
    delete batch;
}

btCollisionShape* lsBulletShape::getShape()
{
	if(! shape )
	{
		// Default shape sphere.
		Make_Sphere( 1.0 );
		return shape;
	} else return shape;
}


