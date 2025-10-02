#include <iostream>
#include <sstream>

#include "../vortx_debug.h"

// Render
#include "../render/lsRender.h"

// Events
#include "../events/lsEventManager.h"
#include "../events/lsEvent_DelNode.h"

#include "lsScene.h"
#include "lsNode.h"


lsNode::lsNode( lsNode* parent )
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsNode::lsNode() START - Name: " << getName() << endl;
#endif
	
	name = "Node";
	node_type = LSNT_NODE;

	this->parent = parent;

	// defaults
	ModelMatrix.LoadIdentity();
	Set_Pos( 0, 0, 0 );
	Set_Rot( 0, 0, 0 );
	Set_Scale( 1.0, 1.0, 1.0 );

	// Life
	life = 0.0;
	timer_life.start_Timer();
}

lsNode::~lsNode()
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsNode::~lsNode() - Name: " << getName() << endl;
#endif
	// Nodes never self deletes
}

void lsNode::Move( __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render, __attribute__((unused)) double delta )
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
	{
		ModelMatrix = parent->ModelMatrix;
	} else
	{
		ModelMatrix.LoadIdentity();
	}
	
	ModelMatrix = ModelMatrix * transmat * rotmat * scalemat;
	
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
 * \param render
 * \param delta
 */
void lsNode::MoveThread( lsScene* scn, lsRender* render, double delta )
{
#ifdef VORTX_DEBUG_NODETHREADS
	cout << "lsNode::MoveThred()" << endl;
#endif
	
	vector<thread> th;
	int threads = 1;
	int tc = 0;
	
	// Threads
	threads = thread::hardware_concurrency();
	
#ifdef VORTX_DEBUG_NODETHREADS
	cout << "lsNode::MoveThread - Thread count : " << threads << endl;
#endif
	
	// cycle through children nodes.
	// Parents Matrix has been set.
	for (auto& child : m_children)
	{
		// Add thread
		th.push_back( thread( &lsNode::Move, child.get(), scn, render, delta ) );
		tc++;

		// check our thread count
		if(( tc >= threads )||( &(*child) == &(*m_children.back()) ))
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
	}
	// Join last threads.
	if(! th.empty() )
	{
		for( auto &t : th )
		{
			t.join();
		}
	}
	
	// pass it on
	for (auto& child : m_children)
	{
		child->MoveThread( scn, render, delta );
	}
}

// used by lsScene nose
void lsNode::InitDrawlist()
{
	// Do nothing
}

// used by Threading
void lsNode::Optimize( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) lsPass* pass, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render )
{
	// Do nothing
}

void lsNode::OptimizeThread( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render )
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsNode::OptimizeThread()" << endl;
#endif

	vector<thread> th;
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
	for (auto& child : m_children)
	{
		// Add thread
		th.push_back( thread( &lsNode::Optimize, child.get(), cam, pass, scn, render ) );
		tc++;
		
		// check our thread count
		if(( tc >= threads )||( &(*child) == &(*m_children.back()) ))
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
	}
	// Join last threads.
	if(! th.empty() )
	{
		for( auto &t : th )
		{
			t.join();
		}
	}
	
	// Pass it on
	for (auto& child : m_children)
	{
		child->OptimizeThread( cam, pass, scn, render );
	}
}

void lsNode::Draw( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) lsPass* pass, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render )
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
	// Set parent pointer for the new child
	node->setParent(this);
	
	// Add to children vector
	m_children.push_back(unique_ptr<lsNode>(node));
}

void lsNode::Del_Node(lsNode* node)
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsNode::Del_Node() - Removing child: " << node->getName() << " from parent: " << getName() << endl;
#endif
	
	if(! node) return;
	
	// Find and remove the node from m_children
	for (auto it = m_children.begin(); it != m_children.end(); ++it)
	{
		if (it->get() == node)
		{
			// Set parent to nullptr to prevent dangling pointer
			node->setParent(nullptr);
			// Remove the unique_ptr from vector
			m_children.erase(it);
			break;
		}
	}
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
		for (auto& child : m_children)
		{
			child->Find_Node( scn, name );
		}
	}
}

void lsNode::Del_Children()
{
	m_children.clear();
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

	// Pass it on to children
	for (auto& child : m_children)
	{
		child->Save();
	}
}

