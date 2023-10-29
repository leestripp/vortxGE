/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#include <iostream>
#include <sstream>

#include "../vortx_debug.h"

// OpenGL
#include "../opengl/lsOpenGL.h"

// Events
#include "../events/lsEventManager.h"
#include "../events/lsEvent_DelNode.h"

#include "lsScene.h"
#include "lsNode.h"


lsNode::lsNode( lsNode* parent )
{
    name = "Node";
    node_type = LSNT_NODE;

    next_node = NULL;
    prev_node = NULL;
    this->parent = parent;
    node_list = NULL;

    // defaults
    ModelMatrix.LoadIdentity();
    Set_Pos( 0, 0, 0 );
    Set_Rot( 0, 0, 0 );
    Set_Scale( 1.0, 1.0, 1.0 );

    // Life
    life = 0.0;
    timer_life.start_Timer();

#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::lsNode() - Name: " << getName() << "  *******************" << endl;
#endif
}

lsNode::~lsNode()
{
#ifdef VORTX_DEBUG_NODES
    // debug
    cout << "lsNode::~lsNode()" << endl;
#endif
    // Nodes never self deletes
}

void lsNode::Move( lsScene* scn, lsOpenGL* opengl, double delta )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Move()" << endl;
#endif

    // Set transform
    lsMatrix4x4 transmat;
    lsMatrix4x4 rotmat;
    lsMatrix4x4 scalemat;

    transmat.TranslateMatrix( position.x, position.y, position.z );
    rotmat.RotateMatrix( rotation.x, 1, 0, 0 );
    rotmat.RotateMatrix( rotation.y, 0, 1, 0 );
    rotmat.RotateMatrix( rotation.z, 0, 0, 1 );
    scalemat.ScaleMatrix( scale.x, scale.y, scale.z );

    if( parent )
        ModelMatrix = parent->ModelMatrix;
    else
        ModelMatrix.LoadIdentity();

    ModelMatrix *= transmat * rotmat * scalemat;
#ifdef VORTX_DEBUG_NODES
    // debug
    ModelMatrix.debug( "lsNode ModelMatrix: " + getName() );
#endif

    // time to die
    // checkLife( scn );

}

/*!
 * \brief lsNode::MoveThread
 *
 * Calls the Move function on all children nodes to update the ModelMatrix.\n
 * Thread count can be set in the base lsScene, if not set then the system default is used.\n
 *
 * \param scn
 * \param opengl
 * \param delta
 */
void lsNode::MoveThread( lsScene* scn, lsOpenGL* opengl, double delta )
{
#ifdef VORTX_DEBUG_NODETHREADS
    cout << "lsNode::MoveThred()" << endl;
#endif

    vector<thread> th;
    lsNode *ptr;
    int threads = 1;
    int tc = 0;

    // Threads
    threads = thread::hardware_concurrency();

#ifdef VORTX_DEBUG_NODETHREADS
    cout << "lsNode::MoveThread - Thread count : " << threads << endl;
#endif

    // cycle through children nodes.
    // Parents Matrix has been set.
    ptr = getNode_list();
    while( ptr )
    {
        // Add thread
        th.push_back( thread( &lsNode::Move, ptr, scn, opengl, delta ) );
        tc++;

        // check our thread count
        if(( tc >= threads )||( ptr->getNext_node() == NULL ))
        {
#ifdef VORTX_DEBUG_NODETHREADS
            // debug
            cout << "lsNode::MoveThread - Threads pushed : " << tc << endl;
#endif
            // Join and wait
            for( auto &t : th )
            {
                t.join();
            }
            // clean up
            th.clear();
            tc = 0;
        }
        ptr = ptr->getNext_node();
    }

    // pass it on
    if( getNode_list() ) getNode_list()->MoveThread( scn, opengl, delta );
    if( getNext_node() ) getNext_node()->MoveThread( scn, opengl, delta );
}

// used by lsScene nose
void lsNode::InitDrawlist()
{
    // Do nothing
}

// used by Threading
void lsNode::Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl )
{
    // Do nothing
}

void lsNode::OptimizeThread( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::OptimizeThread()" << endl;
#endif

    vector<thread> th;
    lsNode *ptr;
    int threads = 1;
    int tc = 0;

    // Threads
    threads = thread::hardware_concurrency();

#ifdef VORTX_DEBUG_NODETHREADS
    stringstream msg;
    msg << "lsNode::OptimizeThread - Thread count : " << threads << endl;
    cout << msg.str();
#endif

    // cycle through child and its next nodes
    ptr = getNode_list();
    while( ptr )
    {
        // Add thread
        th.push_back( thread( &lsNode::Optimize, ptr, cam, pass, scn, opengl ) );
        tc++;

        // check our thread count
        if(( tc >= threads )||( ptr->getNext_node() == NULL ))
        {
#ifdef VORTX_DEBUG_NODETHREADS
            stringstream msg;
            msg << "lsNode::OptimizeThread - Threads pushed : " << tc << endl;
            cout << msg.str();
#endif
            // Join and wait
            for( auto &t : th )
            {
                t.join();
            }
            // clean up
            th.clear();
            tc = 0;
        }
        ptr = ptr->getNext_node();
    }

    // Pass it on
    if( getNode_list() ) getNode_list()->OptimizeThread( cam, pass, scn, opengl );
    if( getNext_node() ) getNext_node()->OptimizeThread( cam, pass, scn, opengl );
}

void lsNode::Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Draw() - Name: " << getName() << endl;
#endif
    // Reimplement
}

// ########################
// Override in Scene

lsScene* lsNode::Get_Scene()
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Get_Scene() - Name: " << getName() << endl;
#endif
    if( parent ) return parent->Get_Scene();
    return NULL;
}

lsEventManager* lsNode::Get_EventManager()
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Get_EventManager() - Name: " << getName() << endl;
#endif
    if( parent ) return parent->Get_EventManager();
    return NULL;
}


void lsNode::checkLife( lsScene *scn )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::checkLife() - Name: " << getName() << endl;
#endif
    if( life == 0.0 ) return;

    if( timer_life.get_ElapsedTime() > life )
    {
        // Dont fire again
        life = 0.0;

        if( scn )
        {
            lsEvent_DelNode* e = new lsEvent_DelNode();
            e->setNode( this );
            scn->Add_Event( e );
        } else
        {
            // Error
            cout << "ERROR: lsNode::checkLife() Scene pte is NULL : " << getName() << endl;
        }
    }
}

void lsNode::Set_Life( double val )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Set_Life() - Name: " << getName() << endl;
#endif
    timer_life.start_Timer();
    life = val;
}


void lsNode::Add_Node( lsNode *node )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Add_Node() - Name: " << node->getName() << "  to Node : " << getName() << endl;
#endif
    if( node_list ) node_list->setPrev_node( node );
    node->setNext_node( node_list );
    node_list = node;
}

void lsNode::Find_Node( lsScene* scn, const string& name )
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Find_Node() - Name: " << name << endl;
#endif
    if( this->name == name )
    {
        scn->setFound_node( this );
    } else
    {
        // pass it one
        if( node_list ) node_list->Find_Node( scn, name );
        if( next_node ) next_node->Find_Node( scn, name );
    }
}

void lsNode::Del_AllChildren()
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Del_AllChildren() - Parent: " << getName() << endl;
#endif
    if( node_list )
    {
        node_list->Del_AllNodes();
        delete node_list;
        node_list = NULL;
    }
}

void lsNode::Del_AllNodes()
{
#ifdef VORTX_DEBUG_NODES
    cout << "lsNode::Del_AllNodes() - Parent: " << getName() << endl;
#endif
    if( node_list )
    {
        node_list->Del_AllNodes();
#ifdef VORTX_DEBUG_NODES
        cout << "Delete Child: " << node_list->getName() << endl;
#endif
        delete node_list;
        node_list = NULL;
    }

    if( next_node )
    {
        next_node->Del_AllNodes();
#ifdef VORTX_DEBUG_NODES
        cout << "Del_AllNodes() Pass it on - Name: " << next_node->getName() << endl;
#endif
        delete next_node;
        next_node = NULL;
    }
}

// Transforms

void lsNode::Set_Pos(float x, float y, float z)
{
    position = lsVector3( x, y, z );
}

void lsNode::Set_Rot(float x, float y, float z)
{
    rotation = lsVector3( x, y, z );
}

void lsNode::Set_Scale(float x, float y, float z)
{
    scale = lsVector3( x, y, z );
}

//###################
// Load and Save

void lsNode::Save()
{
    cout << toString() << endl;

    // Pass it on
    if( getNode_list() ) getNode_list()->Save();
    if( getNext_node() ) getNext_node()->Save();
}

