# Scene Manager
* Holds a list of Scenes that need to be processed.
* Update() - Calls the first scene's Move()
  * which Polls scene events
  * Updates physics
  * Sets Identity Matrix
* Then it calls MoveThread() on each node in the heirarchy.
* You can link more Scenes to the list to keep backgrounds, physics seperate. e.g. Great for particale systems that don't need to interact with the main Scene.

# Scene Nodes

## Scene (Root node)

``` c++
    // Add viewer control
    viewer = new lsViewer( vortxge->getOpenGL(), NULL );
    if( viewer )
    {
        // Init
        viewer->Init();
        vortxge->getControlManager()->Add_Control( viewer );

        // Create viewer scene : World 1000 meters with 10 max proxies
        viewer_scene = new lsScene( 1000.0, 10 );
        if( viewer_scene )
        {
            viewer->getSceneManager()->Add_Scene( viewer_scene );

            // Add camera and objects etc...

            ...

        }// if viewer_scene
    }// if viewer
```

## lsNode
All Scene nodes are subclassed from this class.

## lsAnimNode
If you want to create your own scene nodes with animation support, you need to subclass lsAnimNode.

## Camera
* void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
* void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
* void Set_Orthographic( float cube );

``` c++
	// Add Camera rig
	camera_base = new lsObject( viewer_scene );
	if( camera_base )
	{
		camera_base->setName( "Camera Base" );
		camera_base->Set_Pos( 0, 2, -8 );

		camera = new lsCamera( camera_base );
		if( camera )
		{
			// Camera defaults
			camera->Set_Perspective( 45.0, 1.0, 1.0, 100.0 );

			camera_base->Add_Node( camera );
		}
		viewer_scene->Add_Node( camera_base );
	}
```

## Object
Holds an Asset with VBM (Mesh) data consisting of 1 or more batches.
Can also be used as an Empty (NULL).

Code example above shows one use of an Object as an empty.

<- [Home](Home)

