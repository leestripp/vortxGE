#include <iostream>

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

// datablocks
#include <datablocks/lsBatch.h>
#include <datablocks/lsBatch_Lines.h>
#include <datablocks/lsBatch_Quad.h>
#include <datablocks/lsMesh.h>
#include <datablocks/lsFBO_ColourDepth_8.h>

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
#include <scene/lsCamera.h>
#include <scene/lsLight_Point.h>

// Light manager
#include <lights/lsLightManager.h>

// Animation
#include <animation/lsAnimationManager.h>


#include "demoscene.h"


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
        vortxge->Init();
        setupControls();
        setupTestScene();

    } else
    {
        cout << "ERROR: DemoScene::Init() : Can't create vortxGE instance." << endl;
        return false;
    }
    return true;
}

void DemoScene::Resize( int x, int y, int w, int h )
{
    if(! vortxge ) return;

    vortxge->Resize( x, y, w, h );
}

void DemoScene::setupControls()
{
    cout << "DemoScene::setupControls() called" << endl;

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
        }// if viewer_scene
    }// if viewer

}

void DemoScene::setupTestScene()
{
	cout << "DemoScene::setupTestScene() called" << endl;

    lsLight_Point *point_light;
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

    // Add some Objects
    lsObject *object;

    // Floor
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Floor" );
        object->Set_Scale( 2.0, 2.0, 2.0 );

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

    // Cube 2
    object = new lsObject( viewer_scene );
    if( object )
    {
        object->setName( "Cube" );
        object->Set_Pos( 0, 1, 0 );
        object->Set_Scale( 1.0, 1.0, 1.0 );

        // Load Mesh asset
        lsAsset_Mesh *asset_mesh;
        asset_mesh = viewer->getAssetManager()->CreateAsset_Mesh( "data/meshes/Cube_tex_dns.vbm" );
        asset_mesh->AllocAsset();
        object->Set_Asset( asset_mesh );

        // Set a light
        object->getLightManager()->Add_Light( point_light );
        viewer_scene->Add_Node( object );
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
        move_strafe += lsVector3( cx, cy, cz );
        move_strafe.normalize();
    }
    if( r ) // right
    {
        cx = camera->ModelMatrix.matrix[0];
        cy = camera->ModelMatrix.matrix[1];
        cz = camera->ModelMatrix.matrix[2];
        move_strafe -= lsVector3( cx, cy, cz );
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
        move_walk += lsVector3( cx, cy, cz );
        move_walk.normalize();
    }
    if( b ) // backwards
    {
        cx = camera->ModelMatrix.matrix[8];
        cy = camera->ModelMatrix.matrix[9];
        cz = camera->ModelMatrix.matrix[10];
        move_walk -= lsVector3( cx, cy, cz );
        move_walk.normalize();
    }
    if( f && b ) move_walk = lsVector3(0,0,0);;
    if( !f && !b ) move_walk = lsVector3(0,0,0);;

    lsVector3 dir = move_walk + move_strafe;
    cam_pos += dir * (delta*factor);

    camera_base->Set_Pos( cam_pos.x, cam_pos.y, cam_pos.z  );
}

void DemoScene::rotateCamera( double rot_x, double rot_y, double rot_z )
{
	// X rotates camera base Y
	camera_base->Set_Rot( 0, rot_x, 0 );
	// Y rotates camera tilt X
	camera->Set_Rot( rot_y, 0, 0 );
}

void DemoScene::Update( double delta )
{
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
    // vortxGE Render
    if( vortxge )
    {
        vortxge->Render();
    }
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

