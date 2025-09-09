#pragma once

#include <iostream>
#include <string>
// opengl
#include "../opengl/lsOpenGL.h"

class lsAsset;
class lsAsset_Mesh;
class lsAsset_Quad;
class lsAsset_MeshGuide_Empty;
class lsAsset_MeshGuide_Grid;
class lsAsset_Texture;
class lsAsset_BulletShape_Plane;
class lsAsset_BulletShape_Box;
class lsAsset_BulletShape_Sphere;
class lsAsset_BulletShape_Pill;
class lsAsset_BulletShape_ConvexHull;
class lsAsset_BulletShape_TriangleMesh;

using namespace std;

//! Manage Assets
/**
 * Create Assets. Auto cleanup.
 *
 * */

class lsAssetManager
{
public:
    lsAssetManager( lsOpenGL *opengl );
    virtual ~lsAssetManager();

	// Shared Assets
    lsAsset* Find_Asset( const string& name );

    // Mesh
    lsAsset_Mesh* CreateAsset_Mesh( const string& filename );
    lsAsset_Quad* CreateAsset_Quad();
    // Texture
    lsAsset_Texture* CreateAsset_Texture( const string& filename );
    // Bullet Shapes
    lsAsset_BulletShape_Plane* CreateAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ );
    lsAsset_BulletShape_Box* CreateAsset_BulletShape_Box( float x, float y, float z );
    lsAsset_BulletShape_Pill* CreateAsset_BulletShape_Pill( float radius, float height );
    // Debug Guides
    lsAsset_MeshGuide_Empty* CreateAsset_MeshGuide_Empty();
    lsAsset_MeshGuide_Grid* CreateAsset_MeshGuide_Grid();

    unsigned int getAsset_count() const
    {
        return asset_count;
    }

    lsAsset* getAssetList() const
    {
        return asset_list;
    }

    void setOpenGL( lsOpenGL *opengl )
    {
        ogl = opengl;
    }

    lsOpenGL *getOpenGL() const
    {
        return ogl;
    }

private:
    void Add_Asset( lsAsset *asset );
    void Del_Asset( lsAsset* asset );

    // list
    lsAsset         *asset_list;
    unsigned int 	asset_count;

    // Link to the opengl instance
    lsOpenGL *ogl;

};
