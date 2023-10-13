# Asset Managers

## Create you own
If your dealing with complex scenes or levels, it is best to create your own asset
managers by instanceing lsAssetManager. Just remember to delete the instance so it can free up all
the memory used by your assets.

## Built in
Some vortxGE Controls have an Asset Manager built in. For example the Viewer control.
The core vortxGE instance also has an Asset Manager you can use for persistant global assets.

# Creating Assets
Asset Managers come with CreateAsset member functions that you use to create your assets.
There's nothing stopping you from subclassing lsAssetManager and adding your own CreateAsset members.

## Mesh
* lsAsset_Mesh* CreateAsset_Mesh( const string& filename );
* lsAsset_Quad* CreateAsset_Quad();

## Texture
* lsAsset_Texture* CreateAsset_Texture( const string& filename );

## Bullet Shapes
* lsAsset_BulletShape_Plane* CreateAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ );
* lsAsset_BulletShape_Box* CreateAsset_BulletShape_Box( float x, float y, float z );

## Debug Guides
* lsAsset_MeshGuide_Empty* CreateAsset_MeshGuide_Empty();
* lsAsset_MeshGuide_Grid* CreateAsset_MeshGuide_Grid();


# Shared Assets
* lsAsset* Find_Asset( const string& name );


# Using Assets
Before using your asset you need to call AllocAsset(), this loads any data from disk and gets it ready for use in video ram if needed.

## AllocAsset()

``` c++
    // Cube
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Cube" );
        object->Set_Pos( 2.5, 1, 2 );
        object->Set_Scale( 1.0, 1.0, 1.0 );

        // Load Mesh asset
        lsAsset_Mesh *asset_mesh;
        asset_mesh = viewer->getAssetManager()->CreateAsset_Mesh( "data/meshes/Cube_logo.vbm" );
        asset_mesh->AllocAsset();
        object->Set_Asset( asset_mesh );

        // Set a light
        object->getLightManager()->Add_Light( point_light );
        viewer_scene->Add_Node( object );
    }
```

## FreeAsset()
To free up video and/or system memory being used by an asset call :

``` c++
 asset_mesh->FreeAsset();
```

The asset is still available from the asset manager it was created in,
but to use it again you need to call AllocAsset() to reload the data.

<- [Home](Home)

