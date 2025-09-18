#pragma once

#include <iostream>
#include <string>

// vortxGE
#include "../vortx_debug.h"

// opengl
#include "../opengl/lsOpenGL.h"

// types
const int LSAT_NONE                 = 0;
const int LSAT_TEXTURE              = 1;
const int LSAT_MESH                 = 2;
const int LSAT_BULLETSHAPE_PLANE    = 3;
const int LSAT_BULLETSHAPE_BOX      = 4;
const int LSAT_BULLETSHAPE_SPHERE   = 5;
const int LSAT_BULLETSHAPE_PILL     = 6;
const int LSAT_QUAD                 = 7;
const int LSAT_BULLETSHAPE_CONVEXHULL = 8;
const int LSAT_BULLETSHAPE_TRIMESH    = 9;
// vortxEDIT
const int LSAT_MESHGUIDE_EMPTY      = 1000;
const int LSAT_MESHGUIDE_GRID       = 1001;


class lsAssetManager;
class lsData;

using namespace std;


class lsAsset
{
public:
    lsAsset( lsOpenGL *ogl );
    virtual ~lsAsset();

    // User count
    void AddUser();
    void RemoveUser();

    // data
    virtual void AllocAsset() = 0;
    virtual void FreeAsset() = 0;

    // Asset Navi
    lsAsset *getNext_asset() const
    {
        return next_asset;
    }

    void setNext_asset(lsAsset *next_asset)
    {
        this->next_asset = next_asset;
    }

    lsAsset *getPrev_asset() const
    {
        return prev_asset;
    }

    void setPrev_asset(lsAsset *prev_asset)
    {
        this->prev_asset = prev_asset;
    }

    // Data

    int getAsset_type() const
    {
        return asset_type;
    }

    void setAsset_type( int val )
    {
        asset_type = val;
    }

    const string& getAsset_name()
    {
        return asset_name;
    }

    void setAsset_name( const string& name )
    {
        asset_name = name;
    }

    unsigned int getUsers() const
    {
        return users;
    }

    lsData* getData() const
    {
        return data;
    }

    void setData( lsData* val )
    {
        data = val;
    }

    // Manager

    lsAssetManager* getAssetManager() const
    {
        return assetmanager;
    }

    void setAssetManager( lsAssetManager* val )
    {
        assetmanager = val;
    }

    // opengl
    lsOpenGL* getOpenGL() const
    {
        return opengl;
    }

    void setOpenGL( lsOpenGL* val )
    {
        opengl = val;
    }

private:
    lsAsset* next_asset;
    lsAsset* prev_asset;
    // Managers
    lsAssetManager *assetmanager;
    lsOpenGL *opengl;

    // Data
    int asset_type;
    string asset_name;
    unsigned int users;
    lsData *data;
};
