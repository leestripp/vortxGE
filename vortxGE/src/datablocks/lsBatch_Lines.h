/**
 * @file lsBatch_Lines.h
 * @brief Line batch for wireframe and line primitive rendering
 * 
 * lsBatch_Lines handles rendering of line primitives. Lines are
 * commonly used for:
 * - Wireframe visualization
 * - Debug rendering
 * - Grid overlays
 * - Edge detection visualization
 * - Particle trails
 * 
 * @section line_building Building Lines
 * 
 * Lines are built by calling Begin() to allocate buffers,
 * AddLine() to add line segments, and End() to finalize.
 * 
 * @section usage Usage
 * 
 * @code
 * lsBatch_Lines* lines = new lsBatch_Lines(renderer);
 * lines->Begin(1000);  // Allocate space for 1000 lines
 * 
 * Vector3f verts[2] = {start, end};
 * Vector4f cols[2] = {color1, color2};
 * lines->AddLine(verts, cols);
 * 
 * lines->End();
 * lines->Copy_GLdata();
 * lines->Draw(camera, object, pass);
 * @endcode
 * 
 * @see lsBatch for base class
 * @see lsBatch_Normals for normal visualization
 */

#ifndef vortxGE_lsBatch_Lines_h
#define vortxGE_lsBatch_Lines_h

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"

class lsRender;

/**
 * @class lsBatch_Lines
 * @brief Line batch for wireframe and line rendering
 * 
 * lsBatch_Lines provides line primitive rendering with
 * per-vertex colors. Each line segment has two vertices
 * with independent positions and colors.
 * 
 * @note LS_LINES primitive type
 * @note Supports per-vertex coloring for gradients
 */

class lsBatch_Lines : public lsBatch
{
public:
    /**
     * @brief Construct a lines batch
     * @param render Pointer to the renderer
     */
    lsBatch_Lines( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsBatch_Lines();

    //=========================================================================
    // lsBatch Virtual Methods
    //=========================================================================
    
    /**
     * @brief Draw the lines
     * 
     * @param cam Camera for rendering
     * @param object Object being rendered
     * @param pass Current render pass
     */
    void Draw( lsCamera *cam, lsObject *object, int pass_type );
    
    /**
     * @brief Update batch state
     * 
     * @param delta Time since last frame
     */
    void Update( double delta );

    //=========================================================================
    // Line Building
    //=========================================================================
    
    /**
     * @brief Begin line construction
     * 
     * Allocates buffer space for line vertices.
     * 
     * @param nMaxVerts Maximum number of vertices
     */
    void Begin( lsUInt nMaxVerts );
    
    /**
     * @brief Add a line segment
     * 
     * @param verts Array of 2 vertex positions
     * @param vColours Array of 2 vertex colors
     */
    void AddLine( Vector3f verts[2], Vector4f vColours[2] );
    
    /**
     * @brief Finalize line construction
     */
    void End();
    
    /**
     * @brief Clear all line data
     */
    void Cleanup();

    //=========================================================================
    // Utility Builders
    //=========================================================================
    
    /**
     * @brief Build empty wireframe cube
     * 
     * @param scale Cube size
     */
    void buildEmpty( lsFloat scale );
    
    /**
     * @brief Build grid overlay
     */
    void buildGrid();

    //=========================================================================
    // OpenGL Data Management
    //=========================================================================
    
    /**
     * @brief Upload data to GPU
     */
    void Copy_GLdata();
    
    /**
     * @brief Release GPU resources
     */
    void Clear_GLdata();

private:
    /// Number of vertices
    lsUInt nNumVerts;

    //=========================================================================
    // Buffer Objects
    //=========================================================================
    
    /** @brief Vertex Array Object */
    lsVAOID m_Vao;
    
    /** @brief Vertex position buffer */
    lsBufferID vbo_verts;
    
    /** @brief Vertex color buffer */
    lsBufferID vbo_colours;

    //=========================================================================
    // Host Data
    //=========================================================================
    
    /** @brief Vertex positions */
    Vector3f *pVerts;
    
    /** @brief Vertex colors */
    Vector4f *pColours;

};


#endif
