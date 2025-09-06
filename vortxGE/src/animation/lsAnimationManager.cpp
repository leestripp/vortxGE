// Animation
#include "lsAnimationManager.h"
#include "lsAnimProperty.h"
#include "lsAnimProperty_Vector3.h"
#include "lsAnimProperty_Vector4.h"

// Scene
#include "../scene/lsAnimNode.h"

// datablocks
#include "../datablocks/lsBatch.h"

// properties
#include "../properties/lsProperty.h"

/*!
 * Sets defaults.
 */
lsAnimationManager::lsAnimationManager()
{
    animnode = NULL;
    animproperty_list = NULL;

    framerate = 60.0;
    curr_frame = 1.0;

    // Play
    start_frame = 0.0;
    end_frame = 0.0;
    looping = false;
    pause = true;

    // transforms
    position = NULL;
    rotation = NULL;
    scale = NULL;
    // batch materials
    diffuse = NULL;
    specular = NULL;
    ambient = NULL;
}

/*!
 * Cleans up all properties in the list.
 */
lsAnimationManager::~lsAnimationManager()
{
    if( animproperty_list ) delete animproperty_list;
}

/*!
 * Adds all properties from an lsAnimNode subclass to the manager.
 * This must be called on your node to make animation work.
 */
void lsAnimationManager::Init( lsAnimNode *n, double fps )
{
    animnode = n;
    framerate = fps;
    pause = true;

    lsProperty *ptr;

    // setup property links
    ptr = animnode->getProperty_list();
    while( ptr )
    {
        Add_AnimProperty( ptr );
        ptr = ptr->getNext_prop();
    }
}

void lsAnimationManager::UpdateAll( double delta )
{
    if( pause ) return;

    // current frame
    if( curr_frame < start_frame ) curr_frame = start_frame;
    if( curr_frame > end_frame )
    {
        if( looping )
            curr_frame = start_frame;
        else
            curr_frame = end_frame;

    } else
    {
        curr_frame += (delta / 1000.0)*framerate; // in milisecs
    }
    // debug
    // cout << "lsAnimationManager::UpdateAll : Current frame = " << curr_frame << endl;

    lsAnimProperty* ptr;
    ptr = animproperty_list;
    while( ptr )
    {
        ptr->Update( curr_frame );
        ptr = ptr->getNext_animprop();
    }
}

void lsAnimationManager::Play( bool loop )
{
    looping = loop;
    pause = false;
}

void lsAnimationManager::Stop()
{
    pause = true;
}


// ###########################################################################################

void lsAnimationManager::Add_AnimProperty(lsProperty* p)
{
    lsAnimProperty *prop = NULL;

    switch( p->getProp_type() )
    {
    case LSPROP_VECTOR3:
        prop = new lsAnimProperty_Vector3( p );
        // Fast lookups
        if( p->getProp_id() == LSPROPID_POSITION )
        {
            position = (lsAnimProperty_Vector3*)prop;
        } else if( p->getProp_id() == LSPROPID_ROTATION )
        {
            rotation = (lsAnimProperty_Vector3*)prop;
        } else if( p->getProp_id() == LSPROPID_SCALE )
        {
            scale = (lsAnimProperty_Vector3*)prop;
        }
        break;

    case LSPROP_VECTOR4:
        prop = new lsAnimProperty_Vector4( p );
        // Fast lookups
        if( p->getProp_id() == LSPROPID_DIFFUSE )
        {
            diffuse = (lsAnimProperty_Vector4*)prop;
        } else if( p->getProp_id() == LSPROPID_SPECULAR )
        {
            specular = (lsAnimProperty_Vector4*)prop;
        } else if( p->getProp_id() == LSPROPID_AMBIENT )
        {
            ambient = (lsAnimProperty_Vector4*)prop;
        }
        break;

    default:
        cout << "lsAnimationManager::Add_AnimProperty : ERROR - unknown property" << endl;
        break;
    }

    // Add
    if( prop )
    {
        prop->setNext_animprop( animproperty_list );
        if( animproperty_list ) animproperty_list->setPrev_animprop( prop );
        animproperty_list = prop;
    }
}


// ###########################################################################################

void lsAnimationManager::InsertKey_Pos(double frame, const lsVector3& pos)
{
    if( frame < start_frame ) start_frame = frame;
    if( frame > end_frame ) end_frame = frame;

    position->InsertKey( frame, pos );
}

void lsAnimationManager::InsertKey_Rot(double frame, const lsVector3& rot)
{
    if( frame < start_frame ) start_frame = frame;
    if( frame > end_frame ) end_frame = frame;

    rotation->InsertKey( frame, rot );
}

void lsAnimationManager::InsertKey_Scale(double frame, const lsVector3& aca)
{
    if( frame < start_frame ) start_frame = frame;
    if( frame > end_frame ) end_frame = frame;

    scale->InsertKey( frame, aca );
}


// ###########################################################################################

void lsAnimationManager::InsertKey_Diffuse(double frame, const lsVector4& val)
{
    if( frame < start_frame ) start_frame = frame;
    if( frame > end_frame ) end_frame = frame;

    diffuse->InsertKey( frame, val );
}

void lsAnimationManager::InsertKey_Specular(double frame, const lsVector4& val)
{
    if( frame < start_frame ) start_frame = frame;
    if( frame > end_frame ) end_frame = frame;

    specular->InsertKey( frame, val );
}

void lsAnimationManager::InsertKey_Ambient(double frame, const lsVector4& val)
{
    if( frame < start_frame ) start_frame = frame;
    if( frame > end_frame ) end_frame = frame;

    ambient->InsertKey( frame, val );
}


