// bullet
#include <bullet/btBulletDynamicsCommon.h>

// opengl
#include "../opengl/lsOpenGL.h"

// filesystem
#include "../filesystem/lsFileIO.h"

// datablocks
#include "lsString.h"
#include "lsBatch_Bullet.h"

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
// From vbm file

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

btCollisionShape* lsBulletShape::getShape()
{
	if(! shape )
	{
		// Default shape sphere.
		Make_Sphere( 1.0 );
		return shape;
	} else return shape;
}

