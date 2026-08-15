/**
 * @file lsObject.h
 * @brief Renderable 3D object in the scene
 * 
 * lsObject represents a renderable entity in the 3D world.
 * It combines:
 * - Mesh asset (geometry)
 * - Light manager (per-object lights)
 * - Physics body (optional, for collision/physics)
 * - Animation system
 * 
 * @section usage Usage
 * @code
 * lsObject* obj = new lsObject(scene);
 * obj->Set_Asset(meshAsset);
 * obj->Set_Pos(0, 0, 0);
 * scene->Add_Node(obj);
 * @endcode
 * 
 * @note Remember to call Init_Animation() after creation to enable animation
 * 
 * @see lsAsset_Mesh for mesh assets
 * @see lsAnimNode for animation base
 * @see lsBulletObject for physics
 */

#pragma once

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

// scene
#include "../scene/lsAnimNode.h"
#include "../scene/lsLightManager.h"

//=========================================================================
// Occlusion Types
//=========================================================================

/** @brief No occlusion testing */
const int LSOT_NONE         =	0;
/** @brief Object can occlude other objects */
const int LSOT_OCCLUDER		=	1;

// Forward declarations
class lsAsset;
class lsMesh;
class lsBulletObject;
class lsProperty;
class lsAnimationManager;

/**
 * @class lsObject
 * @brief Renderable 3D object with mesh, lights, and physics
 * 
 * lsObject is the primary class for objects that appear in the scene.
 * It combines multiple components:
 * - Geometry (via mesh asset)
 * - Lighting (per-object light manager)
 * - Physics (optional bullet object)
 * - Animation (inherited from lsAnimNode)
 * 
 * @section rendering Rendering
 * 
 * Objects are rendered through the draw list in lsScene.
 * During rendering:
 * 1. Optimize() performs frustum culling
 * 2. Draw() renders the mesh with appropriate shaders
 * 
 * @section physics Physics Integration
 * 
 * Objects can have physics bodies attached:
 * @code
 * lsBulletObject* body = new lsBulletObject(scene->getBulletworld(), obj);
 * body->Create_Box(1, 1, 1);
 * obj->setBulletObject(body);
 * @endcode
 */
class lsObject : public lsAnimNode
{
public:
    /**
     * @brief Construct an object
     * @param parent Parent node (usually lsScene)
     */
	lsObject( lsNode* parent );
	
    /**
     * @brief Destructor
     * 
     * Cleans up mesh, physics body, and lights.
     */
	virtual ~lsObject();
	
    //=========================================================================
	// Virtual Overrides
	//=========================================================================
	
    /**
     * @brief Update object state
     * 
     * Updates animation and syncs with physics.
     * 
     * @param scn Scene
     * @param render Renderer
     * @param delta Time since last frame
     */
	virtual void Move( lsScene* scn, lsRender* render, double delta );
	
    /**
     * @brief Optimize for rendering
     * 
     * Performs frustum culling and distance sorting.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Scene
     * @param render Renderer
     */
	virtual void Optimize( lsCamera* cam, int pass_type, lsScene* scn, lsRender* render );
	
    /**
     * @brief Render the object
     * 
     * Draws the mesh with appropriate shaders and lights.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Scene
     * @param render Renderer
     */
	virtual void Draw( lsCamera* cam, int pass_type, lsScene* scn, lsRender* render );
	
    /**
     * @brief Render without occlusion testing
     * 
     * Used for shadow map rendering and other passes
     * where occlusion doesn't matter.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Scene
     * @param render Renderer
     */
	void DrawSingle( lsCamera* cam, int pass_type, lsScene* scn, lsRender* render );
	
    //=========================================================================
	// Transform Overrides
	//=========================================================================
	
    /**
     * @brief Set position
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
	void Set_Pos(float x, float y, float z);
	
    /**
     * @brief Set rotation
     * @param x Pitch (degrees)
     * @param y Yaw (degrees)
     * @param z Roll (degrees)
     */
	void Set_Rot(float x, float y, float z);
	
    /**
     * @brief Set scale
     * @param x X scale
     * @param y Y scale
     * @param z Z scale
     */
	void Set_Scale(float x, float y, float z);
	
    //=========================================================================
	// Mesh
	//=========================================================================
	
    /**
     * @brief Get mesh
     * @return Pointer to lsMesh
     */
	lsMesh* Get_Mesh();
	
    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Serialize object
     * @return String representation
     */
	virtual const string& toString();

    //=========================================================================
	// Asset
	//=========================================================================
	
    /**
     * @brief Set mesh asset
     * @param asset Pointer to lsAsset (usually lsAsset_Mesh)
     */
	void Set_Asset( lsAsset* asset );
	
    /**
     * @brief Get mesh asset
     * @return Pointer to asset
     */
	lsAsset* getAsset() const
	{
		return asset;
	}

    //=========================================================================
	// Physics
	//=========================================================================
	
    /**
     * @brief Get physics body
     * @return Pointer to lsBulletObject
     */
	lsBulletObject* getBulletObject() const
	{
		return bulletobject;
	}

    /**
     * @brief Set physics body
     * @param val Pointer to bullet object
     */
	void setBulletObject( lsBulletObject* val )
	{
		bulletobject = val;
	}

    //=========================================================================
	// Lights
	//=========================================================================
	
    /**
     * @brief Get light manager
     * @return Pointer to lsLightManager
     */
	lsLightManager* getLightManager() const
	{
		return lightmanager;
	}

    //=========================================================================
	// Rendering Flags
	//=========================================================================
	
    /**
     * @brief Get mesh culling flag
     * @return true if mesh is culled
     */
	bool getcull_mesh() const
	{
		return cull_mesh;
	}

    /**
     * @brief Set mesh culling
     * @param val true to enable culling
     */
	void setCull_mesh( bool val )
	{
		cull_mesh = val;
	}

    /**
     * @brief Get occlusion status
     * @return true if occluded
     */
	bool getOccluded() const
	{
		return occluded;
	}

    /**
     * @brief Set occlusion status
     * @param val true if occluded
     */
	void setOccluded( bool val )
	{
		occluded = val;
	}

    /**
     * @brief Get distance to camera
     * @return Distance in world units
     */
	float getDistance_to_camera() const
	{
		return distance_to_camera;
	}

    /**
     * @brief Set distance to camera
     * @param val Distance
     */
	void setDistance_to_camera( float val )
	{
		distance_to_camera = val;
	}
	void setMaxDrawDist( float val ) { m_maxDrawDist = val; }
	float getMaxDrawDist() const { return m_maxDrawDist; }

    /**
     * @brief Get occlusion type
     * @return Occlusion type constant
     */
	int getOcclusion_type() const
	{
		return occlusion_type;
	}

    /**
     * @brief Set occlusion type
     * @param val LSOT_* constant
     */
	void setOcclusion_type( int val )
	{
		occlusion_type = val;
	}

    /**
     * @brief Get renderer occlusion query ID
     * @return Renderer query ID (lsQueryID = uint32_t)
     */
	lsQueryID getOcclusionID() const
	{
		return occlusion_id;
	}

    /**
     * @brief Set occlusion query ID
     * @param val Renderer query ID
     */
	void setOcclusionID( lsQueryID val )
	{
		occlusion_id = val;
	}

    //=========================================================================
	// Draw List Navigation
	//=========================================================================
	
    /**
     * @brief Get next object in draw list
     * @return Pointer to next lsObject
     */
	lsObject* getNext_drawobject() const
	{
		return next_drawobject;
	}

    /**
     * @brief Set next object in draw list
     * @param val Pointer to next object
     */
	void setNext_drawobject( lsObject* val )
	{
		next_drawobject = val;
	}

    /**
     * @brief Get previous object in draw list
     * @return Pointer to previous lsObject
     */
	lsObject* getPrev_drawobject() const
	{
		return prev_drawobject;
	}

    /**
     * @brief Set previous object in draw list
     * @param val Pointer to previous object
     */
	void setPrev_drawobject( lsObject* val )
	{
		prev_drawobject = val;
	}

    //=========================================================================
	// Shadow Flags
	//=========================================================================
	
    /**
     * @brief Get shadow casting flag
     * @return true if casts shadows
     */
	bool getCasts_shadows() const { return casts_shadows; }
	
    /**
     * @brief Set shadow casting
     * @param val true to cast shadows
     */
	void setCasts_shadows(bool val) { casts_shadows = val; }
	
    /**
     * @brief Get shadow receiving flag
     * @return true if receives shadows
     */
	bool getReceives_shadows() const { return receives_shadows; }
	
    /**
     * @brief Set shadow receiving
     * @param val true to receive shadows
     */
	void setReceives_shadows(bool val) { receives_shadows = val; }
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Mesh asset */
	lsAsset             *asset;
    
    /** @brief Per-object light manager */
	lsLightManager      *lightmanager;
    
    /** @brief Physics body */
	lsBulletObject      *bulletobject;

	//=========================================================================
	// Draw List
	//=========================================================================
	
    /** @brief Next object in draw list */
	lsObject *next_drawobject;
    
    /** @brief Previous object in draw list */
	lsObject *prev_drawobject;

	//=========================================================================
	// Culling
	//=========================================================================
	
    /** @brief Enable mesh culling */
	bool cull_mesh;
    
    /** @brief Object is occluded */
	bool occluded;
    
    /** @brief Distance to camera for sorting */
	float distance_to_camera;
	/// 0 = unlimited. Distance (camera to bSphere surface) beyond which
	/// the object is culled in Optimize — a REAL cull that skips Draw
	/// entirely (no per-draw shader/light setup). Shadow passes ignore
	/// it (their camera is the light).
	float m_maxDrawDist = 0.0f;
    
    /** @brief Occlusion test type */
	int occlusion_type;
    
    /** @brief OpenGL occlusion query ID */
	lsQueryID occlusion_id;
	
	//=========================================================================
	// Shadows
	//=========================================================================
	
    /** @brief Cast shadows flag */
	bool casts_shadows;
    
    /** @brief Receive shadows flag */
	bool receives_shadows;
};
