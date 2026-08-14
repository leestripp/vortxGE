/**
 * @file lsBatch.h
 * @brief Base class for vertex batch data (VBO)
 * 
 * lsBatch is the foundation for all Vertex Buffer Object (VBO) data
 * in the rendering system. It provides a unified interface for different
 * types of geometry batches including meshes, lines, quads, and physics shapes.
 * 
 * @section batch_types Batch Types
 * 
 * - LSBT_NONE: No geometry
 * - LSBT_LINE: Line primitives
 * - LSBT_QUAD: Quad primitives
 * - LSBT_NORMALS: Normal visualization lines
 * - LSBT_BULLET: Physics mesh (Bullet physics)
 * - LSBT_MESH: General triangle mesh
 * 
 * @section usage Usage
 * 
 * @code
 * lsBatch* batch = new lsBatch_Mesh(renderer);
 * batch->setName("MyMesh");
 * batch->setShader(shader);
 * batch->Copy_GLdata();  // Upload to GPU
 * batch->Draw(camera, object, pass);  // Render
 * @endcode
 * 
 * @see lsBatch_Mesh for triangle meshes
 * @see lsBatch_Quad for quad geometry
 * @see lsBatch_Lines for line rendering
 * @see lsData for base data class
 */

#ifndef vortxGE_lsBatch_h
#define vortxGE_lsBatch_h

#include <iostream>
#include <cstdint>
#include <string>

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

// Math
#include "../math/lsBoundingSphere.h"

// Memory
#include "lsData.h"

class lsObject;
class lsCamera;
class lsShader;

//=========================================================================
// Batch Type Constants
//=========================================================================

/** @brief No geometry */
const int LSBT_NONE         = 0;
/** @brief Line primitives */
const int LSBT_LINE         = 1;
/** @brief Quad primitives */
const int LSBT_QUAD         = 2;
/** @brief Normal visualization */
const int LSBT_NORMALS      = 3;
/** @brief Triangle mesh for Bullet physics */
const int LSBT_BULLET       = 4;
/** @brief General triangle mesh */
const int LSBT_MESH         = 5;

using namespace std;

/**
 * @class lsBatch
 * @brief Base class for VBO geometry batches
 * 
 * lsBatch provides the common interface for all geometry batches.
 * It handles OpenGL buffer management, shader association, and
 * the drawing interface used by the render system.
 * 
 * @note Each batch type implements its own geometry generation
 * @note Batches are typically created by asset loaders
 */

class lsBatch : public lsData
{
public:
    /**
     * @brief Construct a batch
     * @param render Pointer to the renderer
     */
    lsBatch( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsBatch();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Draw the batch geometry
     * 
     * @param cam Camera for rendering
     * @param object Object being rendered
     * @param pass Current render pass
     */
    virtual void Draw( lsCamera *cam, lsObject *object, int pass_type ) = 0;
    
    /**
     * @brief Update batch state
     * 
     * @param delta Time since last frame (seconds)
     */
    virtual void Update( double delta ) = 0;

    //=========================================================================
    // OpenGL Data Management
    //=========================================================================
    
    /**
     * @brief Copy data to GPU
     * 
     * Uploads vertex data to OpenGL buffers.
     * Called after all geometry data is added.
     */
    virtual void Copy_GLdata() = 0;
    
    /**
     * @brief Clear GPU data
     * 
     * Releases OpenGL buffer resources.
     */
    virtual void Clear_GLdata() = 0;

    //=========================================================================
    // Batch Chain Navigation
    //=========================================================================
    
    /**
     * @brief Get next batch in chain
     * @return Pointer to next lsBatch
     */
    lsBatch* getNext_batch() const
    {
        return next_batch;
    }

    /**
     * @brief Set next batch
     * @param val Pointer to next batch
     */
    void setNext_batch( lsBatch *val )
    {
        next_batch = val;
    }

    /**
     * @brief Get previous batch
     * @return Pointer to previous lsBatch
     */
    lsBatch* getPrev_batch() const
    {
        return prev_batch;
    }

    /**
     * @brief Set previous batch
     * @param val Pointer to previous batch
     */
    void setPrev_batch( lsBatch *val )
    {
        prev_batch = val;
    }

    //=========================================================================
    // Properties
    //=========================================================================
    
    /**
     * @brief Get batch type
     * @return LSBT_* constant
     */
    int getBatch_type() const
    {
        return batch_type;
    }

    /**
     * @brief Set batch type
     * @param val LSBT_* constant
     */
    void setBatch_type( int val )
    {
        batch_type = val;
    }

    /**
     * @brief Get batch name
     * @return Name string
     */
    const string& getName()
    {
        return name;
    }

    /**
     * @brief Set batch name
     * @param val Name string
     */
    void setName( const string& val )
    {
        name = val;
    }

    /**
     * @brief Get shader
     * @return Pointer to lsShader
     */
    lsShader* getShader() const
    {
        return shader;
    }

    /**
     * @brief Set shader
     * @param val Pointer to lsShader
     */
    void setShader( lsShader* val )
    {
        shader = val;
    }
	
    /**
     * @brief Get renderer
     * @return Pointer to lsRender
     */
    lsRender *getRender() const;
    
    /**
     * @brief Set renderer
     * @param value Pointer to renderer
     */
    void setOgl(lsRender *value);
	
    /**
     * @brief Get vertex count
     * @return Number of vertices
     */
    int getVertex_count() const;
    
    /**
     * @brief Set vertex count
     * @param value Number of vertices
     */
    void setVertex_count(int value);
	
private:
    /// Next batch in chain
    lsBatch     *next_batch;
    
    /// Previous batch in chain
    lsBatch     *prev_batch;

    /// Renderer reference
    lsRender *m_render;

    /// Batch type (LSBT_*)
    int         batch_type;
    
    /// Batch name
    string      name;
    
    /// Associated shader
    lsShader    *shader;
    
    /// Vertex count
    int         vertex_count;
	
public:
    /// Bounding sphere for culling
    lsBoundingSphere bSphere;
};

#endif
