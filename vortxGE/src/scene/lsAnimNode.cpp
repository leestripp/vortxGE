// Threads
#include <mutex>

// core
#include "../vortx_debug.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector3.h"

// animation
#include "../animation/lsAnimationManager.h"

#include "lsAnimNode.h"

lsAnimNode::lsAnimNode( lsNode* p ) : lsNode(p)
{
    setNode_type( LSNT_ANIMNODE );
    setName("lsAnimNode");

    // properties
    property_list = nullptr; // Changed from NULL

    // Create animation manager
    animationmanager = new lsAnimationManager();
    if(! animationmanager )
    {
        // ERROR
        cout << "lsAnimNode - ERROR: cant create lsAnimationManager instance" << endl;

    } else
    {
        // Add default properties
        lsProperty_Vector3 *pv3;

        pv3 = new lsProperty_Vector3( &position );
        pv3->setProp_name( "Position" );
        pv3->setProp_id( LSPROPID_POSITION );
        Add_Property( pv3 );

        pv3 = new lsProperty_Vector3( &rotation );
        pv3->setProp_name( "Rotation" );
        pv3->setProp_id( LSPROPID_ROTATION );
        Add_Property( pv3 );

        pv3 = new lsProperty_Vector3( &scale );
        pv3->setProp_name( "Scale" );
        pv3->setProp_id( LSPROPID_SCALE );
        Add_Property( pv3 );
    }
}

lsAnimNode::~lsAnimNode()
{
    // self cleanning
    if( property_list ) delete property_list;
    if( animationmanager ) delete animationmanager;

}


void lsAnimNode::Move( lsScene* scn, __attribute__((unused)) lsRender* render, double delta )
{
    // apply animation
    if( getAnimationManager() ) getAnimationManager()->UpdateAll( delta );

    // Set transform
    lsMatrix4x4 transmat;
    lsMatrix4x4 rotmat;
    lsMatrix4x4 scalemat;

    transmat.TranslateMatrix( position.x, position.y, position.z );
    rotmat.RotateMatrix( rotation.x, 1, 0, 0 );
    rotmat.RotateMatrix( rotation.y, 0, 1, 0 );
    rotmat.RotateMatrix( rotation.z, 0, 0, 1 );
    scalemat.ScaleMatrix( scale.x, scale.y, scale.z );

    if( getParent() )
        ModelMatrix = getParent()->ModelMatrix;
    else
        ModelMatrix.LoadIdentity();

    ModelMatrix = ModelMatrix * transmat * rotmat * scalemat;
	
    // Life
    checkLife( scn );
}

void lsAnimNode::Optimize( __attribute__((unused)) lsCamera *cam, __attribute__((unused)) int pass_type, __attribute__((unused)) lsScene *scn, __attribute__((unused)) lsRender *render)
{
}

void lsAnimNode::Draw( __attribute__((unused)) lsCamera *cam, __attribute__((unused)) int pass_type, __attribute__((unused)) lsScene *scn, __attribute__((unused)) lsRender *render)
{
}


// Animation

//! Setup animation system
/*!
 * Must be called before using any animation functions. In your app add some animation data then call this function.
 *
 * */
void lsAnimNode::Init_Animation( double fps )
{
    // setup animation manager for node properties
    animationmanager->Init( this, fps );
}

void lsAnimNode::Play_Animation( bool loop )
{
    animationmanager->Play( loop );
}

void lsAnimNode::Stop_Animation()
{
    animationmanager->Stop();
}


void lsAnimNode::Add_Property( lsProperty* p )
{
    p->setNext_prop( property_list );
    if( property_list ) property_list->setPrev_prop( p );
    property_list = p;
}


// Save data
const string& lsAnimNode::toString()
{
    save_str = "add_animnode";

    return save_str;
}


// ########################
// Override in lsScene

lsScene* lsAnimNode::Get_Scene()
{
    if( getParent() ) return getParent()->Get_Scene();

    return nullptr; // Changed from NULL
}

lsEventManager* lsAnimNode::Get_EventManager()
{
    if( getParent() ) return getParent()->Get_EventManager();

    return nullptr; // Changed from NULL
}
