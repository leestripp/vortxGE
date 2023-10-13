# Animation system
Still under development.

## Catmull-Rom

``` c++

    // Empty for animation
    light_control = new lsObject( viewer_scene );
    if( light_control )
    {
        light_control->setName( "Light anim empty" );
        light_control->Set_Pos( 0, 0, 0 );

        // Test animation
        // We need to init the animation system on objects before we use them.
        light_control->Init_Animation( framerate );
        // Get the manager attached to this object.
        lam = light_control->getAnimationManager();
        // Rotate the object 360 degrees.
        lam->InsertKey_Rot( 1, lsVector3( 0, 0, 0 ) );
        lam->InsertKey_Rot( 1, lsVector3( 0, 0, 0 ) );
        lam->InsertKey_Rot( 100, lsVector3( 0, 359, 0 ) );
        lam->InsertKey_Rot( 100, lsVector3( 0, 359, 0 ) );
        // set to play
        light_control->Play_Animation( true );

        // Attach light
        point_light = new lsLight_Point( light_control );
        if( point_light )
        {
            point_light->Set_Pos( -15, 3, 0 );

            debug_empty = new lsObject( point_light );
            if( debug_empty )
            {
                // Create a debug object
                lsAsset_MeshGuide_Empty *asset_empty;
                asset_empty = viewer->getAssetManager()->CreateAsset_MeshGuide_Empty();
                asset_empty->AllocAsset();
                debug_empty->Set_Asset( asset_empty );

                point_light->Add_Node( debug_empty );
            }
            light_control->Add_Node( point_light );
        }
        viewer_scene->Add_Node( light_control );
    }
```

## Linear
Needs design.

## Constant
Needs design.

<- [Home](Home)

