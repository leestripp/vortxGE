# Controls
The idea of Controls in vortxGE is that they are building blocks for your game, from Menus to Levels or dynamic sandboxes.
Its up to the developer to create these Controls, this way vortxGE isn't dictating gameplay or placing any restrictions on the game.

That said I may add some Controls later that do basic functions like loading a walkthrough scene etc...
This may have a Loader and Menus subclassed from lsControl.

## Viewer
* Use this to combine everything for output, also good for simple scene renders.
* Contains a Pass Manager, Scene Manager and Asset Manager.

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

            // Add camera, lights and objects etc...

            ...

        }// if viewer_scene
    }// if viewer
```

## Composite
* Use textures (FBO) from passes to build final output. But really its up to you.
* Contains a Pass Manager, Scene Manager and Asset Manager.


<- [Home](Home)

