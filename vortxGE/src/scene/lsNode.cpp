/**
 * @file lsNode.cpp
 * @brief Scene graph node implementation
 * 
 * This file implements the lsNode class which is the base class for all
 * scene graph elements. Nodes form a hierarchical tree structure where
 * each node can have children and a single parent.
 * 
 * Key functionality:
 * - Hierarchical transform management (position, rotation, scale)
 * - Parent-child relationships with transform propagation
 * - Multi-threaded transform updates
 * - Lifecycle management (creation, deletion, lifetime)
 * 
 * @see lsNode.h
 * @see lsObject.h
 * @see lsScene.h
 */

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


/**
 * @brief Construct a node with optional parent
 * 
 * Initializes a new node in the scene graph. If parent is specified,
 * the new node becomes a child of that parent.
 * 
 * Default values:
 * - Position: (0, 0, 0)
 * - Rotation: (0, 0, 0) - Euler angles in degrees
 * - Scale: (1, 1, 1)
 * - Active: true
 * - Not marked for deletion
 * 
 * @param parent Optional parent node (can be nullptr)
 */
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

/**
 * @brief Clean up deleted child nodes
 * 
 * Removes children marked for deletion from the children list.
 * This is called during the scene update to safely remove
 * nodes that were marked for deletion during the frame.
 * 
 * The cleanup process:
 * 1. Iterate through all children
 * 2. Check if child is marked for deletion (m_delete flag)
 * 3. If marked, remove parent reference and erase from vector
 * 4. Otherwise, continue to next child
 * 
 * @note Uses iterator invalidation-safe erase pattern
 * @note Does not immediately delete - lets unique_ptr handle memory
 */
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

/**
 * @brief Update node transform (Model Matrix)
 * 
 * Computes the world model matrix for this node based on:
 * 1. Local position, rotation, and scale
 * 2. Parent's model matrix (if parent exists)
 * 
 * The transform is computed as:
 * ModelMatrix = ParentModelMatrix * Translation * Rotation * Scale
 * 
 * Transform order (applied right to left):
 * 1. Scale first (affects size)
 * 2. Rotate second (around local axes)
 * 3. Translate last (moves to world position)
 * 
 * This order ensures scaling and rotation happen relative to
 * the object's local origin before translation moves it.
 * 
 * @param scn Pointer to scene (unused, for interface compatibility)
 * @param render Pointer to renderer (unused)
 * @param delta Time since last frame (seconds)
 */

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

/**
 * @brief Multi-threaded transform update for children
 * 
 * Updates transform matrices for all child nodes using parallel
 * execution. This exploits multi-core CPUs by distributing
 * the transform calculation across multiple threads.
 * 
 * Threading strategy:
 * 1. Detect available hardware threads (thread::hardware_concurrency())
 * 2. Collect children (filtering out deleted ones)
 * 3. Launch up to N threads where N = CPU core count
 * 4. Each thread calls Move() on one child
 * 5. Join threads before returning
 * 
 * The parent matrix must be computed BEFORE calling this method
 * (it's already set by the parent's Move call).
 * 
 * This enables hierarchical transform updates in parallel:
 * Parent Matrix -> Child1, Child2, Child3... (in parallel)
 * 
 * @param scn Pointer to scene
 * @param render Pointer to renderer
 * @param delta Time since last frame (seconds)
 * 
 * @note Thread count is hardware-dependent
 * @note Children without parent reference are still processed
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
void lsNode::Optimize( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) int pass_type, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render )
{
	// Do nothing
}

void lsNode::OptimizeThread( lsCamera* cam, int pass_type, lsScene* scn, lsRender* render )
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
			th.push_back( thread( &lsNode::Optimize, child.get(), cam, pass_type, scn, render ) );
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
			child->OptimizeThread( cam, pass_type, scn, render );
		}
	}
}

void lsNode::Draw( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) int pass_type, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsRender* render )
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
		// pass_type it one
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
