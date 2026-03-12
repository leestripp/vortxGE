/**
 * @file lsNode.h
 * @brief Base class for all scene graph nodes
 * 
 * lsNode is the fundamental building block of the scene graph.
 * All visible and invisible objects in the engine inherit from this class.
 * It provides:
 * - Hierarchical parent-child relationships
 * - Transform operations (position, rotation, scale)
 * - Lifecycle management (active, deletion)
 * - Thread-safe operations
 * - Virtual methods for rendering and updating
 * 
 * @section nodetypes Node Types
 * 
 * The engine defines various node types via constants:
 * - LSNT_NODE: Base node type
 * - LSNT_OBJECT: Renderable 3D object
 * - LSNT_CAMERA: Camera viewpoint
 * - LSNT_LIGHT: Light source
 * - LSNT_SCENE: Scene container
 * - LSNT_FIRSTPERSONRIG: First-person controller
 * - LSNT_ANIMNODE: Animated node
 * - And more...
 * 
 * @see lsObject for renderable objects
 * @see lsScene for the root scene container
 * @see lsCamera for cameras
 * @see lsLight for lights
 */

#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>

// math
#include "../math/lsMatrix4x4.h"
#include "../math/lsVector3.h"

// timers
#include "../timers/lsTimer.h"

//=========================================================================
// Node Type Constants
//=========================================================================

/** @brief Base node type */
const int LSNT_NODE             =	0;
/** @brief Renderable 3D object */
const int LSNT_OBJECT           =	1;
/** @brief Terrain object */
const int LSNT_TERRAIN          =	2;
/** @brief Camera viewpoint */
const int LSNT_CAMERA           =	4;
/** @brief Scene container */
const int LSNT_SCENE            =	5;
/** @brief Motion/Animation path */
const int LSNT_MOTION           =	6;
/** @brief Vehicle controller */
const int LSNT_VEHICLE          =	7;
/** @brief First-person camera rig */
const int LSNT_FIRSTPERSONRIG   =	8;
/** @brief Zone/area */
const int LSNT_ZONE         	 =  	9;
/** @brief Debug object */
const int LSNT_DEBUG        	  =   10;
/** @brief Actor/character */
const int LSNT_ACTOR        	  =   11;
/** @brief Light source */
const int LSNT_LIGHT            =	12;
/** @brief Empty/placeholder node */
const int LSNT_EMPTY            =	13;
/** @brief LOD (Level of Detail) object */
const int LSNT_OBJECT_LOD       =	14;
/** @brief Object emitter */
const int LSNT_OBJECT_EMITTER	=	15;
/** @brief Particle system emitter */
const int LSNT_PARTICLE_EMITTER	=	16;
/** @brief Individual particle */
const int LSNT_PARTICLE         =	17;
/** @brief Animated node */
const int LSNT_ANIMNODE         =	18;

// Game items
/** @brief Trigger volume */
const int LSNT_TRIGGER          =	100;
/** @brief Pickup item */
const int LSNT_PICKUP           =	101;

// Forward declarations
class lsScene;
class lsCamera;
class lsLight;
class lsPass;
class lsRender;
class lsEventManager;

using namespace std;

/**
 * @class lsNode
 * @brief Base class for all scene graph nodes
 * 
 * lsNode provides the foundation for the scene graph hierarchy.
 * Every object in the scene (meshes, lights, cameras, etc.) is a node
 * that can have children and be transformed in 3D space.
 * 
 * @section transform Transforms
 * 
 * Each node has:
 * - position: 3D position in local space (relative to parent)
 * - rotation: 3D rotation angles (degrees)
 * - scale: 3D scale factors
 * - ModelMatrix: Combined transformation matrix
 * 
 * @section lifecycle Lifecycle
 * 
 * Nodes can be:
 * - Active (m_active): Being processed by engine
 * - Marked for deletion (m_delete): Will be removed next frame
 * 
 * @section threading Thread Safety
 * 
 * Some operations support threading:
 * - MoveThread(): Update position/physics in separate thread
 * - OptimizeThread(): Frustum culling in separate thread
 * 
 * @note This class uses std::unique_ptr for automatic child memory management
 */
class lsNode
{
public:
    /**
     * @brief Construct a node with optional parent
     * @param parent Parent node (can be nullptr for root)
     * 
     * If parent is provided, this node is automatically added
     * to the parent's children list.
     */
	lsNode( lsNode* parent );
	
    /**
     * @brief Virtual destructor
     * 
     * Cleans up children first, then releases resources.
     * Virtual to allow derived class cleanup.
     */
	virtual ~lsNode();
	
	//=========================================================================
	// Virtual Override Methods
	//=========================================================================
	
    /**
     * @brief Update node for current frame
     * 
     * Called every frame to update node state. Override in
     * derived classes to implement custom behavior.
     * 
     * @param scn Pointer to the scene
     * @param render Pointer to the renderer
     * @param delta Time since last frame in seconds
     */
	virtual void Move( lsScene* scn, lsRender* render, double delta );
	
    /**
     * @brief Optimize node for rendering
     * 
     * Performs view frustum culling and other optimizations.
     * Override in derived classes for custom optimization.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Pointer to the scene
     * @param render Pointer to the renderer
     */
	virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
	
    /**
     * @brief Render the node
     * 
     * Called during the rendering pass. Override in derived
     * classes to implement custom rendering.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Pointer to the scene
     * @param render Pointer to the renderer
     */
	virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
	
    /**
     * @brief Initialize draw list
     * 
     * Called once before first render to build internal
     * display lists or prepare for rendering.
     */
	virtual void InitDrawlist();
	
	//=========================================================================
	// Thread Operations
	//=========================================================================
	
    /**
     * @brief Run Move() in a separate thread
     * 
     * For parallelizing physics or animation updates.
     * Thread joins after completion.
     * 
     * @param scn Pointer to the scene
     * @param render Pointer to the renderer
     * @param delta Time since last frame
     */
	void MoveThread( lsScene* scn, lsRender* render, double delta );
	
    /**
     * @brief Run Optimize() in a separate thread
     * 
     * For parallelizing frustum culling operations.
     * Thread joins after completion.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Pointer to the scene
     * @param render Pointer to the renderer
     */
	void OptimizeThread( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );

    //=========================================================================
	// Scene Access
	//=========================================================================
    
    /**
     * @brief Get the scene this node belongs to
     * @return Pointer to lsScene, or nullptr if not in a scene
     */
	virtual lsScene *Get_Scene();
	
    /**
     * @brief Get the event manager for this node's scene
     * @return Pointer to lsEventManager
     */
	virtual lsEventManager *Get_EventManager();

	//=========================================================================
	// Lifecycle Management
	//=========================================================================
	
    /**
     * @brief Check and process node lifecycle
     * 
     * Checks if node has exceeded its lifetime and marks
     * for deletion if lifetime has expired.
     * 
     * @param scn Pointer to the scene
     */
	void checkLife( lsScene *scn );
	
    /**
     * @brief Set the lifetime of this node
     * 
     * Node will be automatically marked for deletion
     * after this many seconds.
     * 
     * @param val Lifetime in seconds (0 = infinite)
     */
	void Set_Life( double val );
	
    /**
     * @brief Cleanup deleted child nodes
     * 
     * Removes all children marked for deletion from
     * the children vector.
     */
	void CleanupDeletedNodes();
	
	//=========================================================================
	// Flags
	//=========================================================================
	
    /**
     * @brief Set the active state of this node
     * 
     * Inactive nodes are skipped during scene updates
     * and rendering.
     * 
     * @param active true to activate, false to deactivate
     */
	void SetActive(bool active);
	
    /**
     * @brief Get the active state
     * @return true if active, false if inactive
     */
	bool GetActive() const;
	
    /**
     * @brief Mark this node for deletion
     * 
     * Node will be cleaned up at end of frame.
     * Children are also marked for deletion.
     */
	void MarkForDeletion();
	
    /**
     * @brief Check if marked for deletion
     * @return true if marked for deletion
     */
	bool IsMarkedForDeletion() const;
	
	//=========================================================================
	// Node Hierarchy
	//=========================================================================
	
    /**
     * @brief Add a child node to this node
     * 
     * The child node's parent pointer is set to this node.
     * 
     * @param node Pointer to child node (takes ownership)
     */
	void Add_Node(lsNode* node);
	
    /**
     * @brief Remove a child node
     * 
     * Removes specified child from hierarchy.
     * Does not delete the node - caller must handle that.
     * 
     * @param node Pointer to child to remove
     */
	void Del_Node( lsNode *node );
	
    /**
     * @brief Find a node by name in this subtree
     * 
     * Recursively searches children for a node with
     * the specified name.
     * 
     * @param scn Pointer to the scene (for event system)
     * @param name Name of node to find
     */
	void Find_Node( lsScene* scn, const string& name );
	
    /**
     * @brief Delete all children
     * 
     * Removes and deletes all child nodes.
     */
	void Del_Children();
	
	//=========================================================================
	// Transform Operations
	//=========================================================================
	
    /**
     * @brief Set local position
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
	virtual void Set_Pos(float x, float y, float z);
	
    /**
     * @brief Set local rotation (Euler angles)
     * 
     * Rotation order is typically YXZ (Yaw, Pitch, Roll).
     * 
     * @param x Rotation around X axis (degrees)
     * @param y Rotation around Y axis (degrees)
     * @param z Rotation around Z axis (degrees)
     */
	virtual void Set_Rot(float x, float y, float z);
	
    /**
     * @brief Set local scale
     * @param x Scale factor for X
     * @param y Scale factor for Y
     * @param z Scale factor for Z
     */
	virtual void Set_Scale(float x, float y, float z);
	
    /**
     * @brief Rotate around world axis
     * 
     * Applies rotation in world space (not local).
     * 
     * @param angle Rotation angle in degrees
     * @param x X component of rotation axis
     * @param y Y component of rotation axis
     * @param z Z component of rotation axis
     */
	void RotateWorld(float angle, float x, float y, float z);
	
	//=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Save node data
     * 
     * Serializes node properties to internal string buffer.
     * Override in derived classes to save custom data.
     */
	void Save();
	
    /**
     * @brief Get string representation for serialization
     * @return String containing node data
     */
	virtual const string& toString() = 0;
	
    /** @brief Internal buffer for serialized data */
	string save_str;
	
	//=========================================================================
	// Navigation / Accessors
	//=========================================================================

    /**
     * @brief Get parent node
     * @return Pointer to parent, or nullptr if root
     */
	lsNode* getParent() const
	{
		return parent;
	}

    /**
     * @brief Set parent node
     * @param val New parent node
     */
	void setParent( lsNode* val )
	{
		parent = val;
	}
	
    /**
     * @brief Get children (const access)
     * @return Const reference to children vector
     */
    const vector<unique_ptr<lsNode>>& getChildren() const {
        return m_children;
    }
    
    /**
     * @brief Get children (mutable access)
     * @return Reference to children vector
     */
    vector<unique_ptr<lsNode>>& getChildren() {
        return m_children;
    }
	
	//=========================================================================
	// Data Access
	//=========================================================================
	
    /**
     * @brief Get node name
     * @return Reference to name string
     */
	const string& getName()
	{
		return name;
	}
	
    /**
     * @brief Set node name
     * @param val New name
     */
	void setName( const string& val )
	{
		name = val;
	}
	
    /**
     * @brief Get node type constant
     * @return Node type (LSNT_* constant)
     */
	int getNode_type() const
	{
		return node_type;
	}
	
    /**
     * @brief Set node type
     * @param val Node type constant
     */
	void setNode_type( int val )
	{
		node_type = val;
	}
	
    /**
     * @brief Set active state (alias for SetActive)
     * @param val true to activate
     */
	void setActive(bool val) { m_active = val; }
	
    /**
     * @brief Get active state
     * @return true if active
     */
	bool getActive() const { return m_active; }
	
	//=========================================================================
	// Public Data Members
	//=========================================================================
	
    /**
     * @brief Combined model transformation matrix
     * 
     * Cached transformation matrix combining position,
     * rotation, and scale. Updated each frame.
     */
	lsMatrix4x4 ModelMatrix;
	
    /**
     * @brief Local position relative to parent
     */
	lsVector3 position;
	
    /**
     * @brief Local rotation angles (degrees)
     */
	lsVector3 rotation;
	
    /**
     * @brief Local scale factors
     */
	lsVector3 scale;
	
private:
    /**
     * @brief Parent node (nullptr if root)
     */
	lsNode  *parent;
	
    /**
     * @brief Child nodes - owned by this node
     * 
     * Using unique_ptr for automatic memory management.
     * Children are deleted when parent is destroyed.
     */
	vector<unique_ptr<lsNode>> m_children;
	
    /**
     * @brief Node name for identification
     */
	string name;
	
    /**
     * @brief Node type constant (LSNT_*)
     */
	int node_type;
	
    /**
     * @brief Active flag - node should be processed
     */
	bool m_active;
    
    /**
     * @brief Delete flag - node scheduled for deletion
     */
    bool m_delete;
	
	// Lifecycle
	
    /**
     * @brief Timer for lifetime tracking
     */
	lsTimer timer_life;
	
    /**
     * @brief Lifetime in seconds (0 = infinite)
     */
	double life;
};
