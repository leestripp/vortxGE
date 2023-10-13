# Render Passes
Passes in vortxGE are used to set openGL's viewport and setup what your rendering to.
Examples are DR, FBO and Occlusion. DR is standard framebuffer. FBO is a frame buffer object.
Occlusion is used to find hidden objects that don't need to be rendered, this speeds up Draws.

## Direct Render

``` c++
	// create pass
	lsPass_DR *pass_dr = new lsPass_DR( vortxge->getOpenGL() );
	if( pass_dr )
	{
		pass_dr->setBackground_colour( lsVector4(0.5,0,0,1.0) );

		viewer->getPassManager()->Add_Pass( pass_dr );
		pass_dr->setScene( viewer_scene );
		pass_dr->setCamera( camera );
		pass_dr->setCull_camera( camera );

	}// if pass_dr
```

## FBO

## Occlusion


<- [Home](Home)

