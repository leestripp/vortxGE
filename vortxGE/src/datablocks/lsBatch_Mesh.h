/**
 * @file lsBatch_Mesh.h
 * @brief Triangle mesh batch for 3D geometry rendering
 * 
 * lsBatch_Mesh handles the most common type of geometry in 3D rendering:
 * triangle meshes. It manages vertex data including positions, normals,
 * tangents, texture coordinates, and indices for efficient GPU rendering.
 * 
 * @section buffer_types Buffer Types
 * 
 * - VERTEX_DATA: Vertex positions
 * - COLOUR_DATA: Vertex colors
 * - NORMAL_DATA: Surface normals
 * - TANGENT_DATA: Tangent vectors (for normal mapping)
 * - UV_DATA: Texture coordinates
 * - INDEX_DATA: Element indices
 * 
 * @section material_properties Material Properties
 * 
 * The batch supports PBR (Physically Based Rendering) material properties:
 * - m_albedo: Base color (RGB)
 * - m_roughness: Surface roughness (0-1)
 * - m_metallic: Metallic factor (0-1)
 * 
 * @section textures Textures
 * 
 * Supports multiple texture maps:
 * - tex_albedo: Base color/diffuse map
 * - tex_roughness_metallic: Combined roughness/metallic map
 * - tex_normal: Normal map for surface detail
 * 
 * @see lsBatch for base class
 * @see lsShader for shader management
 * @see lsAsset_Mesh for mesh asset loading
 */

#pragma once

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"

//=========================================================================
// Buffer ID Constants
//=========================================================================

/** @brief Vertex position buffer */
const int VERTEX_DATA			= 0;
/** @brief Vertex color buffer */
const int COLOUR_DATA			= 1;
/** @brief Vertex normal buffer */
const int NORMAL_DATA			= 2;
/** @brief Vertex tangent buffer */
const int TANGENT_DATA			= 3;
/** @brief Texture coordinate buffer */
const int UV_DATA			    = 4;
/** @brief Element index buffer */
const int INDEX_DATA				= 5;

class lsRender;
class lsObject;
class lsPass;
class lsCamera;
class lsProperty;
class lsAnimationManager;
class lsListItem_vertexdata;

/**
 * @class lsBatch_Mesh
 * @brief Triangle mesh batch for 3D rendering
 * 
 * lsBatch_Mesh manages the vertex data for triangle meshes.
 * It handles buffer allocation, data uploading to GPU, and
 * rendering with support for PBR materials and animations.
 * 
 * @section usage Usage
 * 
 * @code
 * lsBatch_Mesh* mesh = new lsBatch_Mesh(renderer);
 * mesh->Init_data(vertexCount, indexCount);
 * // Add vertices, normals, UVs, indices...
 * mesh->Copy_GLdata();  // Upload to GPU
 * mesh->Draw(camera, object, pass);  // Render
 * @endcode
 * 
 * @note Supports hardware instancing for efficient rendering
 * @note Animation properties can be added for skeletal animation
 */

class lsBatch_Mesh : public lsBatch
{
public:
    /**
     * @brief Construct a mesh batch
     * @param render Pointer to the renderer
     */
	lsBatch_Mesh( lsRender *render );
	
    /**
     * @brief Destructor
     */
	virtual ~lsBatch_Mesh();

	//=========================================================================
    // lsBatch Virtual Methods
    //=========================================================================
	
    /**
     * @brief Draw the mesh
     * 
     * Renders the mesh using OpenGL draw calls.
     * 
     * @param cam Camera for rendering
     * @param object Object being rendered
     * @param pass Current render pass
     */
	void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
	
    /**
     * @brief Update batch state
     * 
     * @param delta Time since last frame
     */
	void Update( double delta );

	//=========================================================================
    // Data Management
    //=========================================================================
	
    /**
     * @brief Initialize vertex data buffers
     * 
     * @param vc Maximum vertex count
     * @param fit Maximum index count
     */
	void Init_data( int vc, int fit );
	
    /**
     * @brief Add vertex data
     * 
     * @param vd Vertex data to add
     */
	void Add_VertexData( lsListItem_vertexdata *vd );
	
    /**
     * @brief Add index
     * 
     * @param index Index value to add
     */
	void Add_Index( int index );

	//=========================================================================
    // OpenGL Data Management
    //=========================================================================
	
    /**
     * @brief Upload data to GPU
     * 
     * Transfers all vertex and index data to OpenGL buffers.
     */
	void Copy_GLdata();
	
    /**
     * @brief Release GPU resources
     */
	void Clear_GLdata();

	//=========================================================================
    // Animation
    //=========================================================================
	
    /**
     * @brief Add animation property
     * 
     * @param p Property to add
     */
	void Add_Property( lsProperty* p );

    //=========================================================================
    // Bullet Physics Access
    //=========================================================================
    
    /**
     * @brief Get vertex array
     * @return Array of vertex positions
     */
    Vector3f* getVerts() const { return pVerts; }
    
    /**
     * @brief Get index array
     * @return Array of indices
     */
    std::uint16_t* getIndices() const { return pIndex; }
    
    /**
     * @brief Get vertex count
     * @return Number of vertices
     */
    int getVertCount() const { return count_verts; }
    
    /**
     * @brief Get index count
     * @return Number of indices
     */
    int getIndexCount() const { return count_tris; }
	
	//=========================================================================
    // Material Properties
    //=========================================================================
	
    /** @brief Albedo color (RGBA) */
	lsVector4 m_albedo;
	
    /** @brief Roughness factor (0-1) */
	float m_roughness;
	
    /** @brief Metallic factor (0-1) */
	float m_metallic;
	
	//=========================================================================
    // Textures
    //=========================================================================
	
    /** @brief Albedo/diffuse texture */
	lsTextureID tex_albedo;
	
    /** @brief Combined roughness/metallic texture */
	lsTextureID tex_roughness_metallic;
	
    /** @brief Normal map texture */
	lsTextureID tex_normal;
	
private:
    /// Vertex Array Object
	lsVAOID vao_mesh;
	
    /// OpenGL buffer objects
	lsBufferID bufferObjects[6];

	//=========================================================================
    // Vertex Data
    //=========================================================================
	
    /** @brief Vertex positions */
	Vector3f *pVerts;
	
    /** @brief Vertex normals */
	Vector3f *pNormals;
	
    /** @brief Vertex tangents */
	Vector3f *pTangents;
	
    /** @brief Texture coordinates */
	Vector2f *pTexCoords;
	
    /** @brief Element indices */
	std::uint16_t *pIndex;
	
    /** @brief Current vertex index for adding data */
	int vertex_index;
	
    /** @brief Current triangle index for adding data */
	int tri_index;
	
    /** @brief Total vertex count */
	int count_verts;
	
    /** @brief Total triangle/index count */
	int count_tris;

	//=========================================================================
    // Animation
    //=========================================================================
	
    /** @brief Animation manager for skeletal animation */
	lsAnimationManager *animationmanager;
	
    /** @brief Property list for animation */
	lsProperty *property_list;
};
