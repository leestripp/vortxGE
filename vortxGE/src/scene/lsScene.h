/**
 * @file lsScene.h
 * @brief Scene container - root of the scene graph
 * 
 * lsScene is the root container for all objects in a 3D world.
 * It inherits from lsNode, allowing it to participate in the
 * scene graph hierarchy while also managing:
 * - Physics world (Bullet physics integration)
 * - Asset loading
 * - Light management
 * - Event system
 * - Audio listener
 * - Skybox rendering
 * 
 * Each application typically has one main scene, though
 * multiple scenes can exist (for loading screens, menus, etc.)
 * 
 * @section scenegraph Scene Graph
 * 
 * lsScene acts as the root node of the scene graph:
 * @code
 * lsScene* scene = new lsScene(render, 1000.0f, 1024);
 * 
 * // Add objects as children
 * scene->Add_Node(myObject);
 * 
 * // Get draw list for rendering
 * lsObject* drawList = scene->getDraw_list();
 * @endcode
 * 
 * @see lsSceneManager for scene lifecycle management
 * @see lsNode for base node functionality
 * @see lsObject for renderable objects
 * @see lsBulletWorld for physics
 */

#pragma once

#include <thread>
#include <mutex>

#include "lsNode.h"

// Forward declarations
class lsBulletWorld;
class lsEvent;
class lsObject;
class lsAssetManager;
class lsLightManager;
class lsListener;

/**
 * @class lsScene
 * @brief Scene container - manages all objects in a 3D world
 * 
 * lsScene is the central manager for a 3D world. It contains:
 * - All visible objects in the scene graph
 * - Physics simulation world
 * - Light sources
 * - Event queue
 * - Audio listener position
 * - Skybox/environment
 * 
 * @section physics Physics Integration
 * 
 * The scene maintains a bullet physics world for realistic
 * physics simulation. Objects can be synchronized between
 * the scene graph and physics world.
 * 
 * @section rendering Rendering Pipeline
 * 
 * During rendering:
 * 1. Scene->Move() updates all objects
 * 2. Scene->Optimize() performs culling
 * 3. Scene->Draw() renders visible objects
 * 
 * @note lsScene is subclassed from lsNode, so it can be
 *       added to other scenes as a child (for sub-scenes)
 */
class lsScene : public lsNode
{
public:
    /**
     * @brief Construct a new scene
     * @param m_render Pointer to the renderer
     * @param world_size Size of the physics world (bounding box)
     * @param maxProxies Maximum number of physics proxies
     * 
     * Initializes physics world, asset manager, light manager,
     * event manager, and audio listener.
     */
    lsScene( lsRender* m_render, float world_size, int maxProxies );
    
    /**
     * @brief Destructor
     * 
     * Cleans up physics world, event manager, and all children.
     */
    virtual ~lsScene();
	
    //=========================================================================
    // Virtual Overrides (from lsNode)
    //=========================================================================
    
    /**
     * @brief Update all scene objects
     * 
     * Called each frame to update:
     * - All child nodes (via traverse)
     * - Physics simulation
     * - Event processing
     * 
     * @param scn Pointer to scene (unused, scene is 'this')
     * @param render Pointer to renderer
     * @param delta Time since last frame
     */
    void Move( lsScene* scn, lsRender* render, double delta );
    
    /**
     * @brief Optimize scene for rendering
     * 
     * Performs view frustum culling and other
     * optimizations on all child nodes.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Pointer to scene
     * @param render Pointer to renderer
     */
    void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );

    /** @brief Set the streaming manager (M3) — nullptr disables streaming.
     *  When set, Optimize() drives node-distance asset residency each frame.
     *  Not owned by the scene; caller manages lifetime. */
    void setStreamingManager( class lsStreamingManager* sm ) { m_streamingManager = sm; }
    lsStreamingManager* getStreamingManager() const { return m_streamingManager; }
    
    /**
     * @brief Render the scene
     * 
     * Draws all visible objects via draw_list traversal.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Pointer to scene
     * @param render Pointer to renderer
     */
    void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
    
    /**
     * @brief Initialize rendering resources
     * 
     * Called once before first render to prepare
     * display lists and GPU resources.
     */
    void InitDrawlist();
	
    //=========================================================================
    // Scene Access
    //=========================================================================
    
    /**
     * @brief Get the scene (returns this)
     * 
     * Required by lsNode interface. Since lsScene IS
     * the scene, returns 'this'.
     * 
     * @return Pointer to this scene
     */
    lsScene *Get_Scene();
    
    /**
     * @brief Get the event manager
     * @return Pointer to root event manager
     */
    lsEventManager *Get_EventManager();
    
    //=========================================================================
    // Scene Management
    //=========================================================================
    
    /**
     * @brief Clear all objects from scene
     * 
     * Removes all children, clears draw list,
     * resets physics world.
     */
    void ClearScene();
    
    /**
     * @brief Add object to draw list
     * 
     * Objects in the draw list are rendered each frame.
     * Usually called when creating new objects.
     * 
     * @param object Pointer to object to add
     */
    void Add_DrawObject( lsObject* object );
	
    //=========================================================================
    // Event System
    //=========================================================================
    
    /**
     * @brief Add event to queue
     * 
     * Events are processed in order during Update().
     * 
     * @param event Pointer to event to queue
     * @see Poll_Event()
     */
    void Add_Event( lsEvent* event );
    
    /**
     * @brief Get next event from queue
     * 
     * Retrieves and removes the oldest event from queue.
     * 
     * @return Pointer to next event, or nullptr if empty
     */
    lsEvent* Poll_Event();

    //=========================================================================
    // Serialization
    //=========================================================================
    
    /**
     * @brief Serialize scene to string
     * @return String containing scene data
     */
    virtual const string& toString();
	
    //=========================================================================
    // Accessors
    //=========================================================================
	
    /**
     * @brief Get the draw list head
     * 
     * The draw list is a linked list of objects to render.
     * 
     * @return Pointer to first object in draw list
     */
    lsObject* getDraw_list() const
    {
        return draw_list;
    }
	
    /**
     * @brief Get last found node
     * 
     * Updated by Find_Node() calls.
     * 
     * @return Pointer to found node, or nullptr
     */
    lsNode* getFound_node() const
    {
        return found_node;
    }
	
    /**
     * @brief Set the found node (internal)
     * @param val Node to set as found
     */
    void setFound_node( lsNode* val )
    {
        found_node = val;
    }

    //=========================================================================
    // Physics
    //=========================================================================
	
    /**
     * @brief Get physics world
     * 
     * Access the Bullet physics world for
     * rigid body simulation.
     * 
     * @return Pointer to bullet world
     */
    lsBulletWorld* getBulletworld() const
    {
        return bulletworld;
    }

    /**
     * @brief Set physics world
     * @param val Pointer to bullet world
     */
    void setBulletworld( lsBulletWorld *val )
    {
        bulletworld = val;
    }

    /**
     * @brief Get asset manager
     * 
     * Used for loading meshes, textures, etc.
     * 
     * @return Pointer to asset manager
     */
    lsAssetManager* getAssetManager() const
    {
        return m_assetManager;
    }

    /**
     * @brief Get light manager
     * 
     * Manages all light sources in the scene.
     * 
     * @return Pointer to light manager
     */
    lsLightManager* getLightManager() const
    {
        return m_lightManager;
    }

    //=========================================================================
    // Skybox
    //=========================================================================
    
    /**
     * @brief Get skybox object
     * 
     * The skybox is rendered behind all other objects.
     * 
     * @return Pointer to skybox object, or nullptr
     */
    lsObject* getSkyObject() const { return sky_object; }
    
    /**
     * @brief Set skybox object
     * @param val Pointer to skybox object
     */
    void setSkyObject(lsObject* val) { sky_object = val; }
    
    //=========================================================================
    // Audio
    //=========================================================================
    
    /**
     * @brief Get audio listener
     * 
     * Represents the player's "ears" in 3D space.
     * 
     * @return Pointer to audio listener
     */
    lsListener* getListener() const { return m_listener; }
	
private:
    //=========================================================================
    // Private Members
    //=========================================================================
    
    /**
     * @brief Draw list - objects to render
     */
    lsObject* draw_list;

    /**
     * @brief Event manager for this scene
     */
    lsEventManager *root_eventmanager;
    
    /**
     * @brief Bullet physics world
     */
    lsBulletWorld   *bulletworld;
    
    /**
     * @brief Asset manager for resource loading
     */
    lsAssetManager* m_assetManager;
    
    /**
     * @brief Light manager for scene lights
     */
    lsLightManager* m_lightManager;
    lsStreamingManager* m_streamingManager = nullptr;	///< optional, not owned (M3)
    
    /**
     * @brief Node found during search
     */
    lsNode* found_node;
	
    /**
     * @brief Skybox/environment object
     */
    lsObject* sky_object;
    
    /**
     * @brief Audio listener for 3D sound
     */
    lsListener* m_listener;
};
