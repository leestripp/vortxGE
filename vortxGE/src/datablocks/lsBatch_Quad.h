/**
 * @file lsBatch_Quad.h
 * @brief Quad batch for 2D geometry and screen-space rendering
 * 
 * lsBatch_Quad creates a simple 2x2 unit quad (two triangles) that
 * is commonly used for:
 * - Full-screen quad rendering
 * - Post-processing effects
 * - UI elements
 * - Skyboxes
 * - Light projection surfaces
 * 
 * @section geometry Quad Geometry
 * 
 * The quad is centered at the origin and spans from (-1, -1) to (1, 1)
 * in local space. It can be scaled and positioned using transforms.
 * 
 * @code
 * Unit Quad Layout:
 * (-1, 1) -------- (1, 1)
 *    |              |
 *    |    (0,0)     |
 *    |      +       |
 *    |              |
 * (-1,-1) -------- (1,-1)
 * @endcode
 * 
 * @section usage Usage
 * 
 * @code
 * lsBatch_Quad* quad = new lsBatch_Quad(renderer);
 * quad->Copy_GLdata();  // Upload to GPU
 * quad->Draw(camera, object, pass);  // Render
 * @endcode
 * 
 * @see lsBatch for base class
 * @see lsAsset_Quad for quad asset
 */

#ifndef vortxGE_lsBatch_Quad_h
#define vortxGE_lsBatch_Quad_h

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"

// Forward declaration
class lsTexture;

/**
 * @class lsBatch_Quad
 * @brief Quad batch for 2D rendering
 * 
 * lsBatch_Quad provides a simple quad geometry optimized for
 * screen-space and 2D rendering operations. The quad is
 * unit-sized (2x2) and centered at the origin.
 * 
 * @note Ideal for post-processing and full-screen effects
 * @note UV coordinates range from (0,0) to (1,1)
 */

class lsBatch_Quad : public lsBatch
{
public:
    /**
     * @brief Construct a quad batch
     * @param render Pointer to the renderer
     */
    lsBatch_Quad( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsBatch_Quad();

    //=========================================================================
    // lsBatch Virtual Methods
    //=========================================================================
    
    /**
     * @brief Draw the quad
     * 
     * Renders the quad using indexed triangle draw call.
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

    //=========================================================================
    // Texture Access
    //=========================================================================
    
    /**
     * @brief Get albedo texture
     * @return OpenGL texture ID
     */
    GLuint getTex_albedo() const
    {
        return tex_albedo;
    }

    /**
     * @brief Set albedo texture
     * @param val OpenGL texture ID
     */
    void setTex_albedo( GLuint val )
    {
        tex_albedo = val;
    }

private:
    /// Vertex Array Object
    GLuint vao_quad;
	
    //=========================================================================
    // Buffer Objects
    //=========================================================================
	
    /** @brief Vertex buffer */
	GLuint vbo_verts;
	
    /** @brief UV coordinate buffer */
	GLuint vbo_uvs;
	
    /** @brief Index buffer */
	GLuint vbo_indices;
	
    //=========================================================================
    // Textures
    //=========================================================================
	
    /** @brief Albedo/diffuse texture */
    GLuint      tex_albedo;
};


#endif
