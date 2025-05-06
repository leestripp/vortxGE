#include <iostream>

// GLEW
#include <GL/glew.h>
// vortxGE
#include <vortxGE.h>

// Events
#include <events/lsEventManager.h>
#include <events/lsEvent_HitPoint.h>

// Math
#include <math/lsVector4.h>
#include <math/lsVector3.h>

// Shaders
#include <shader/lsShaderManager.h>
#include <shader/lsShader.h>

// Assets
#include <assets/lsAssetManager.h>
#include <assets/lsAsset.h>
#include <assets/lsAsset_Mesh.h>
#include <assets/lsAsset_Quad.h>
#include <assets/lsAsset_MeshGuide_Empty.h>
#include <assets/lsAsset_MeshGuide_Grid.h>
// bullet
#include <assets/lsAsset_BulletShape_Box.h>
#include <assets/lsAsset_BulletShape_Plane.h>

// Physics
#include <physics/lsBulletObject.h>
#include <physics/lsBulletWorld.h>

// datablocks
#include <datablocks/lsBatch.h>
#include <datablocks/lsBatch_Lines.h>
#include <datablocks/lsBatch_Quad.h>
#include <datablocks/lsMesh.h>
#include <datablocks/lsFBO_ColourDepth_8.h>
#include <datablocks/lsBulletShape.h>

// Controls
#include <controls/lsControlManager.h>
#include <controls/lsControl.h>
#include <controls/lsComposite.h>
#include <controls/lsViewer.h>

// Passes
#include <passes/lsPassManager.h>
#include <passes/lsPass.h>
#include <passes/lsPass_DR.h>
#include <passes/lsPass_FBO.h>
#include <passes/lsPass_Occlusion.h>

// Scene
#include <scene/lsSceneManager.h>
#include <scene/lsNode.h>
#include <scene/lsScene.h>
#include <scene/lsObject.h>
#include <scene/lsObjectEmitter.h>
#include <scene/lsCamera.h>
#include <scene/lsLight_Point.h>

// Light manager
#include <lights/lsLightManager.h>

// Animation
#include <animation/lsAnimationManager.h>


#include "demoscene.h"


// Callback for OpenGL errors.
void GLAPIENTRY MessageCallback( GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			__attribute__((unused)) GLsizei length,
			const GLchar* message,
			__attribute__((unused)) const void* userParam )
{
	cerr << "ERROR: " << ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "unknown" );
	cerr << ", message: " << message;
	cerr << ", source: " << source;
	cerr << ", id: " << id;
	cerr << ", severity: " << severity;
	cerr << ", id: " << id;
	cerr << endl;
}


DemoScene::DemoScene()
{
    camera = NULL;
    camera_base = NULL;

    // Starting pos
    cam_pos = lsVector3( 0, 1.7, -7.5 );
}

DemoScene::~DemoScene()
{
	if (vortxge)
    {
    	vortxge->Cleanup();
    	delete vortxge;
	}
}


/* ********************
 * Setup
 * */

bool DemoScene::Init()
{
    cout << "DemoScene::Init() called" << endl;

    // create vortxge - must have an active GL context
    vortxge = new vortxGE();
    if( vortxge )
    {
        // start GLEW extension handler
        glewExperimental = GL_TRUE;
        glewInit();

        // Need debug context and extensions loaded first.
        glEnable( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( MessageCallback, 0 );

        vortxge->Init();

        // Set some openGL defaults
        glFrontFace( GL_CCW );
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );

        setupControls();
        setupTestScene();

    } else
    {
        cout << "ERROR: DemoScene::Init() : Can't create vortxGE instance." << endl;
        return false;
    }

    // Check GL errors
    vortxge->getOpenGL()->CheckGLErrors( "DemoScene::Init() END" );

    return true;
}

void DemoScene::Resize( int x, int y, int w, int h )
{
    // cout << "DemoScene::Resize" << endl;

    if(! vortxge ) return;
    vortxge->Resize( x, y, w, h );
}

// ************************************
//  Controls
// ************************************

void DemoScene::setupControls()
{
    cout << "DemoScene::setupControls() called" << endl;

    // pass_fbo needs to pass its texture to this.
    lsAsset_Quad *asset_quad = NULL;

    // Add quad viewer control
    lsViewer *quad_viewer = new lsViewer( vortxge->getOpenGL(), NULL );
    if( quad_viewer )
    {
        quad_viewer->setName( "Quad Viewer" );

        // Init
        quad_viewer->Init();
        vortxge->getControlManager()->Add_Control( quad_viewer );

        // Create quad scene
        lsScene *quad_scene = new lsScene( 100.0, 1 );
        if( quad_scene )
        {
            quad_viewer->getSceneManager()->Add_Scene( quad_scene );

            // Add Camera
            lsCamera *quad_camera = new lsCamera( quad_scene );
            if( quad_camera )
            {
                // Orthographic camera defaults
                quad_camera->Set_Orthographic( 1.0 );

                quad_scene->Add_Node( quad_camera );
            }

            // Create Quad
            lsObject *quad = new lsObject( quad_scene );
            if( quad )
            {
                quad->setName( "Quad" );
                quad->Set_Rot( 0, 180, 0 ); // Rotate to face camera.

                // Use quad asset
                asset_quad = quad_viewer->getAssetManager()->CreateAsset_Quad();
                asset_quad->AllocAsset();
                quad->Set_Asset( asset_quad );

                quad_scene->Add_Node( quad );
            }

            // Create Direct render Pass
            lsPass_DR *pass_dr = new lsPass_DR( vortxge->getOpenGL() );
            if( pass_dr )
            {
                pass_dr->setBackground_colour( lsVector4( 0, 0, 0.5, 1.0 ) );

                quad_viewer->getPassManager()->Add_Pass( pass_dr );
                pass_dr->setScene( quad_scene );
                pass_dr->setCamera( quad_camera );
                pass_dr->setCull_camera( quad_camera );
            }
        }// if( quad_scene )
    }// if quad_viewer


    // Add demoscene viewer control
    viewer = new lsViewer( vortxge->getOpenGL(), NULL );
    if( viewer )
    {
        viewer->setName( "Demo Scene Viewer" );

        // Init
        viewer->Init();
        vortxge->getControlManager()->Add_Control( viewer );

        // Create viewer scene :
        // World size 1000 meters
        // maxProxies - Set this high enough for your physics objects or they wont be added to the sim.
        viewer_scene = new lsScene( 1000.0, 1000 );
        if( viewer_scene )
        {
            viewer->getSceneManager()->Add_Scene( viewer_scene );

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

            // create FBO pass
            lsPass_FBO *pass_fbo = new lsPass_FBO( vortxge->getOpenGL() );
            if( pass_fbo )
            {
                // Create FBO textures at FHD 1920x1080 8bpp
                lsFBO_ColourDepth_8 *lsfbo = new lsFBO_ColourDepth_8( 1920, 1080, vortxge->getOpenGL() );
                if( lsfbo )
                {
                    pass_fbo->setFBO( lsfbo );

                    // Connect the Quad
                    asset_quad->Set_texture( lsfbo->getTexture() );
                }

                pass_fbo->setBackground_colour( lsVector4( 0, 0, 0, 1.0 ) );

                viewer->getPassManager()->Add_Pass( pass_fbo );
                pass_fbo->setScene( viewer_scene );
                pass_fbo->setCamera( camera );
                pass_fbo->setCull_camera( camera );

            }// if pass_fbo
        }// if viewer_scene
    }// if viewer

}


// ************************************
//  Test Scene
// ************************************

void DemoScene::setupTestScene()
{
	cout << "DemoScene::setupTestScene() called" << endl;

    lsAnimationManager *lam;
    lsObject *debug_empty;

    // Empty for animation
    light_control = new lsObject( viewer_scene );
    if( light_control )
    {
        light_control->setName( "Light anim empty" );
        light_control->Set_Pos( 0, 0, 0 );

        // Test animation
        // We need to init the animation system on objects before we use them.
        light_control->Init_Animation( 60.0 );
        // Get the manager attached to this object.
        lam = light_control->getAnimationManager();
        // Rotate the object 360 degrees.
        lam->InsertKey_Rot( 1, lsVector3( 0, 0, 0 ) );
        lam->InsertKey_Rot( 1, lsVector3( 0, 0, 0 ) );
        lam->InsertKey_Rot( 240, lsVector3( 0, 359, 0 ) );
        lam->InsertKey_Rot( 240, lsVector3( 0, 359, 0 ) );
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

    // Add some test Objects
    lsObject *object;

    // Floor
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Floor" );

        // Load Mesh asset
        lsAsset_Mesh *asset_mesh;
        asset_mesh = viewer->getAssetManager()->CreateAsset_Mesh( "data/meshes/Floor.vbm" );
        asset_mesh->AllocAsset();
        object->Set_Asset( asset_mesh );

        // Set a light
        object->getLightManager()->Add_Light( point_light );
        viewer_scene->Add_Node( object );
    }

    // Monkey
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Monkey" );
        object->Set_Pos( -2.5, 1, 2 );
        object->Set_Scale( 1.0, 1.0, 1.0 );

        // Load Mesh asset
        lsAsset_Mesh *asset_mesh;
        asset_mesh = viewer->getAssetManager()->CreateAsset_Mesh( "data/meshes/Monkey.vbm" );
        asset_mesh->AllocAsset();
        object->Set_Asset( asset_mesh );

        // Set a light
        object->getLightManager()->Add_Light( point_light );
        viewer_scene->Add_Node( object );
    }

    // Cube 1
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Cube1" );
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

    // Object emitter
    setupObjectEmitter();

}


void DemoScene::setupObjectEmitter()
{
    lsObject *object;

    // Floor Collision
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Floor Collsion" );

        // Create bullet shape
        lsAsset_BulletShape_Plane *asset_bsp = viewer->getAssetManager()->CreateAsset_BulletShape_Plane( 0, 1, 0 );
        if( asset_bsp )
        {
            asset_bsp->AllocAsset();

            lsBulletShape *bulletshape;
            bulletshape = (lsBulletShape *)asset_bsp->getData();

            // create a bullet object
            lsBulletObject *bo = new lsBulletObject( object, bulletshape->getShape(), viewer_scene->getBulletworld()->getMax_proxies() );

            // Connects it all up and checks btcollisonWorld before adding ridged body.
            bo->Init( 0.0 ); // 0 = static
        }
        viewer_scene->Add_Node( object );
    }

    // Load Mesh asset
    lsAsset_Mesh *asset_mesh;
    asset_mesh = viewer->getAssetManager()->CreateAsset_Mesh( "data/meshes/Cube_tex_dns.vbm" );
    if( asset_mesh )
    {
        asset_mesh->AllocAsset();
    } else return;

    // Create bullet shape
    lsAsset_BulletShape_Box *asset_bs = viewer->getAssetManager()->CreateAsset_BulletShape_Box( 1.0, 1.0, 1.0 );
    if( asset_bs )
    {
        asset_bs->AllocAsset();
    } else return;

    // Add emitter to scene
    lsObjectEmitter *oe = new lsObjectEmitter( asset_mesh, asset_bs, viewer_scene );
    if( oe )
    {
        oe->setName( "Object Emitter" );
        oe->Set_Pos( 0, 20, 30 );

        oe->setEmit_life( 20 * 1000 ); // in ms
        oe->setEmittime( 100 );	// in ms
        oe->setEmit_light( point_light );

        viewer_scene->Add_Node( oe );
    }
}

void DemoScene::moveCamera( bool f, bool b, bool l, bool r, double delta, double factor )
{
    float cx, cy, cz;

    // Strafe
    if( l ) // left
    {
        cx = camera->ModelMatrix.matrix[0];
        cy = camera->ModelMatrix.matrix[1];
        cz = camera->ModelMatrix.matrix[2];
        move_strafe = move_strafe + lsVector3( cx, cy, cz );
        move_strafe.normalize();
    }
    if( r ) // right
    {
        cx = camera->ModelMatrix.matrix[0];
        cy = camera->ModelMatrix.matrix[1];
        cz = camera->ModelMatrix.matrix[2];
        move_strafe = move_strafe - lsVector3( cx, cy, cz );
        move_strafe.normalize();
    }
    if( l && r ) move_strafe = lsVector3(0,0,0);;
    if( !l && !r ) move_strafe = lsVector3(0,0,0);;

    // Walk
    if( f ) // forward
    {
        cx = camera->ModelMatrix.matrix[8];
        cy = camera->ModelMatrix.matrix[9];
        cz = camera->ModelMatrix.matrix[10];
        move_walk = move_walk + lsVector3( cx, cy, cz );
        move_walk.normalize();
    }
    if( b ) // backwards
    {
        cx = camera->ModelMatrix.matrix[8];
        cy = camera->ModelMatrix.matrix[9];
        cz = camera->ModelMatrix.matrix[10];
        move_walk = move_walk - lsVector3( cx, cy, cz );
        move_walk.normalize();
    }
    if( f && b ) move_walk = lsVector3(0,0,0);;
    if( !f && !b ) move_walk = lsVector3(0,0,0);;

    lsVector3 dir = move_walk + move_strafe;
    cam_pos = cam_pos + dir * (delta*factor);

    camera_base->Set_Pos( cam_pos.x, cam_pos.y, cam_pos.z  );
}

void DemoScene::rotateCamera( double rot_x, double rot_y, __attribute__((unused)) double rot_z )
{
	// X rotates camera base Y
	camera_base->Set_Rot( 0, rot_x, 0 );
	// Y rotates camera tilt X
	camera->Set_Rot( rot_y, 0, 0 );
}

void DemoScene::Update( double delta )
{
    // cout << "DemoScene::Update" << endl;

    // process scene events
    lsEvent *e;
    while( (e = viewer_scene->Poll_Event()) )
    {
        processEvent( e );
    }

    // vortxGE update
    if( vortxge )
    {
        vortxge->Update( delta );
    }
}

void DemoScene::Render()
{
    // cout << "DemoScene::Render START" << endl;

    // vortxGE Render
    if( vortxge )
    {
        vortxge->Render();
    }

    // cout << "DemoScene::Render END" << endl;
}

/*
 * Event Manager flags the event for deletion and deletes the event for us later.
 */
void DemoScene::processEvent( lsEvent* event )
{
    switch( event->getEvent_type() )
    {
    case LSET_HIT_POINT:
        lsEvent_HitPoint *hit = (lsEvent_HitPoint*)event;
        if( hit )
        {
            // TODO: Create a debug empty at the hit point.
        }
        break;
    }
}

