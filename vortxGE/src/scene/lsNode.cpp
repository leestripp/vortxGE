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
	// flags
	m_active = true;
	m_delete = false;
	
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

void lsNode::CleanupDeletedNodes()
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsScene::CleanupDeletedNodes() - Cleaning up deleted nodes" << endl;
#endif
	
	// Clean up deleted nodes
	for (auto it = m_children.begin(); it != m_children.end();)
	{
		if ((*it)->IsMarkedForDeletion())
		{
			// Set parent to nullptr for the node being removed.
			// The unique_ptr will handle deletion of the node object when it goes out of scope from the vector.
			(*it)->setParent(nullptr);
			it = m_children.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// Flags

void lsNode::SetActive(bool active)
{
	m_active = active;
}

bool lsNode::GetActive() const
{
	return m_active;
}

void lsNode::MarkForDeletion()
{
	m_delete = true;
}

bool lsNode::IsMarkedForDeletion() const
{
	return m_delete;
}

//**************
// Move

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
	
	// Check lift timer.
	checkLife(scn);
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
	
    // Collect children that are not marked for deletion
    vector<lsNode*> children_to_process; // Use local copy
    {
        // For now, assume we're safe because this is part of a Move operation that happens
        // sequentially per node. But be cautious about shared access.
        for (auto& child : m_children)
        {
            if (! child->IsMarkedForDeletion())
            {
                children_to_process.push_back( child.get() );
            }
        }
    }
	
	if(! children_to_process.empty())
	{
		// Cycle through children nodes (filter out deleted ones).
		// Parents Matrix has been set.
		for (auto child : children_to_process)
		{
			// Add thread
			th.push_back( thread( &lsNode::Move, child, scn, render, delta ) );
			tc++;
			
			// check our thread count
			if( tc >= threads )
			{
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
		
		// Pass it on to non-deleted children
		for (auto child : children_to_process)
		{
			child->MoveThread( scn, render, delta );
		}
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

	// cycle through child and its next nodes (filter out deleted ones)
	for (auto& child : m_children)
	{
		if (!child->IsMarkedForDeletion())
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
	}
	// Join last threads.
	if(! th.empty() )
	{
		for( auto &t : th )
		{
			t.join();
		}
	}
	
	// Pass it on to non-deleted children
	for (auto& child : m_children)
	{
		if (!child->IsMarkedForDeletion())
		{
			child->OptimizeThread( cam, pass, scn, render );
		}
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


void lsNode::Add_Node( lsNode* node_ptr )
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsNode::Add_Node() - Name: " << node_ptr->getName() << "  to Node : " << getName() << endl;
#endif
	// Set parent pointer for the new child
	node_ptr->setParent(this);
	
	// Add to children vector
	m_children.push_back( unique_ptr<lsNode>(node_ptr) );
}

void lsNode::Del_Node(lsNode* node)
{
#ifdef VORTX_DEBUG_NODES
	cout << "lsNode::Del_Node() - Removing child: " << node->getName() << " from parent: " << getName() << endl;
#endif
	
	if(! node) return;
	
	// Mark for deletion instead of immediate removal
	node->MarkForDeletion();
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

/*!
 * \brief lsNode::RotateWorld
 *
 * Rotates the node in world space by calculating the new local rotation matrix.
 *
 * \param angle The angle in degrees to rotate.
 * \param x The X component of the rotation axis.
 * \param y The Y component of the rotation axis.
 * \param z The Z component of the rotation axis.
 */
void lsNode::RotateWorld(float angle, float x, float y, float z)
{
	lsMatrix4x4 parentWorldRotationInverse;
	lsMatrix4x4 currentWorldRotation;
	lsMatrix4x4 newWorldRotation;
	lsMatrix4x4 worldRotationDelta;
	lsMatrix4x4 newLocalRotation;
	
	// 1. Get the current world rotation of this node
	// ModelMatrix should be up-to-date from the last Move() call.
	// We extract only the rotation component, discarding translation and scale.
	currentWorldRotation = ModelMatrix.extractRotation();

	// 2. Create the desired world-space rotation matrix (R_delta)
	worldRotationDelta.LoadIdentity();
	// Note: RotateMatrix on an identity matrix creates a pure rotation matrix
	worldRotationDelta.RotateMatrix(angle, x, y, z); // angle is in degrees, RotateMatrix expects degrees for input.
	
	// 3. Calculate the new World Rotation (R_world_new = R_delta * R_world_old)
	// Multiplying R_delta on the left applies the rotation in world space.
	newWorldRotation = worldRotationDelta * currentWorldRotation;
	
	// 4. Calculate the new Local Rotation (R_local_new = (R_parent_rot)^T * R_world_new)
	if( parent )
	{
		// a) Get parent's world rotation (R_parent_rot)
		lsMatrix4x4 parentWorldRotation = parent->ModelMatrix.extractRotation();
		
		// b) Get the transpose (inverse for orthonormal rotation matrix) of parent's world rotation
		parentWorldRotationInverse.transpose(parentWorldRotation);
		
		// c) Calculate new local rotation matrix (local_new = parent_inverse * world_new)
		newLocalRotation = parentWorldRotationInverse * newWorldRotation;
	}
	else
	{
		// No parent, local rotation is the new world rotation
		newLocalRotation = newWorldRotation;
	}

	// 5. Convert the newLocalRotation matrix back to ZYX Euler angles (Roll, Pitch, Yaw) and apply.
	lsVector3 newEulerAngles = lsMatrix4x4::ToEulerZYX(newLocalRotation);
	
	// Update the local rotation members which will be used in the next Move() call.
	// These are typically stored in degrees.
	Set_Rot(newEulerAngles.x, newEulerAngles.y, newEulerAngles.z);
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
