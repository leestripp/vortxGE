/**
 * @file lsLight.h
 * @brief Light source class for 3D scene lighting
 * 
 * lsLight represents a light source in the scene. It inherits from
 * lsCamera because lights use view matrices for shadow mapping.
 * 
 * @section light_types Light Types
 * 
 * Supported light types (LSLT_*):
 * - LSLT_NONE: No light (disabled)
 * - LSLT_POINT: Point light (omnidirectional)
 * - LSLT_SPOT: Spot light (cone-shaped)
 * - LSLT_DIRECT: Directional light (parallel rays)
 * 
 * @section usage Usage
 * @code
 * lsLight* light = new lsLight(scene);
 * light->setLight_type(LSLT_POINT);
 * light->Set_Pos(0, 5, 0);
 * light->diffuse = lsVector4(1, 1, 1, 1);
 * light->setIntensity(1.0f);
 * scene->Add_Node(light);
 * @endcode
 * 
 * @see lsLight_Point for point lights
 * @see lsLight_Spot for spot lights
 * @see lsLight_Direct for directional lights
 * @see lsLightManager for managing scene lights
 */

#pragma once

// Render
#include "../render/lsRender.h"

// math
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"
#include "../math/lsMatrix4x4.h"

#include "lsCamera.h"

//=========================================================================
// Light Type Constants
//=========================================================================

/** @brief No light (disabled) */
const int LSLT_NONE     = 0;
/** @brief Point light (omnidirectional) */
const int LSLT_POINT    = 1;
/** @brief Spot light (cone-shaped) */
const int LSLT_SPOT     = 2;
/** @brief Directional light (parallel rays) */
const int LSLT_DIRECT   = 3;

/**
 * @class lsLight
 * @brief Light source for scene illumination
 * 
 * lsLight provides various light types for scene lighting.
 * Lights are used by shaders to calculate illumination
 * on surfaces.
 * 
 * @section shadow_mapping Shadow Mapping
 * 
 * Lights can generate shadow maps for shadow casting:
 * - buildShadowmap_matrix() creates the shadow projection
 * - tex_shadowmap holds the shadow texture
 * - ShadowmapMatrix transforms world to shadow texture coords
 * 
 * @section colors Light Colors
 * 
 * - ambient: Base illumination (always applied)
 * - diffuse: Color when hitting diffuse surfaces
 * - specular: Color of highlights (shininess)
 */
class lsLight : public lsCamera
{
public:
    /**
     * @brief Construct a light
     * @param parent Parent node (usually lsScene)
     */
    lsLight( lsNode* parent );
    
    /**
     * @brief Destructor
     */
    virtual ~lsLight();

    //=========================================================================
	// Virtual Overrides
	//=========================================================================
	
    /**
     * @brief Optimize light for rendering
     * 
     * Performs shadow map setup if enabled.
     * 
     * @param cam Active camera
     * @param pass Current render pass
     * @param scn Scene
     * @param render Renderer
     */
    virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
    
    /**
     * @brief Build shadow map projection matrix
     * 
     * Creates the matrix that transforms world coordinates
     * to shadow map texture coordinates.
     */
    void buildShadowmap_matrix();
	
    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Serialize light data
     * @return String representation
     */
    virtual const string& toString();

    //=========================================================================
	// Light Properties
	//=========================================================================
	
    /**
     * @brief Set light type
     * @param val LSLT_* constant
     */
    void setLight_type( int val )
    {
        light_type = val;
    }

    /**
     * @brief Get light type
     * @return LSLT_* constant
     */
    int getLight_type() const
    {
        return light_type;
    }

    /**
     * @brief Get light intensity
     * @return Intensity multiplier
     */
    float getIntensity() const
    {
        return intensity;
    }

    /**
     * @brief Set light intensity
     * @param val Intensity (1.0 = normal)
     */
    void setIntensity(float val)
    {
        intensity = val;
    }

    //=========================================================================
	// Shadow Mapping
	//=========================================================================
	
    /**
     * @brief Get shadow map texture
     * @return Renderer texture ID (lsTextureID = uint32_t)
     */
    lsTextureID getTex_shadowmap() const
    {
        return tex_shadowmap;
    }

    /**
     * @brief Set shadow map texture
     * @param val Renderer texture ID
     */
    void setTex_shadowmap( lsTextureID val )
    {
        tex_shadowmap = val;
    }

    /**
     * @brief Check if light has shadow map
     * @return true if shadow map enabled
     */
    bool getHas_shadowmap() const
    {
        return has_shadowmap;
    }

    /**
     * @brief Enable/disable shadow map
     * @param val true to enable shadows
     */
    void setHas_shadowmap(bool val)
    {
        has_shadowmap = val;
    }

    //=========================================================================
	// Shadow Map Resolution
	//=========================================================================
	
    /**
     * @brief Set shadow map X resolution
     * @param val Width in pixels
     */
    void setPixel_x( float val )
    {
        pixel_x = val;
    }

    /**
     * @brief Get shadow map X resolution
     * @return Width in pixels
     */
    float getPixel_x() const
    {
        return pixel_x;
    }

    /**
     * @brief Set shadow map Y resolution
     * @param val Height in pixels
     */
    void setPixel_y( float val )
    {
        pixel_y = val;
    }

    /**
     * @brief Get shadow map Y resolution
     * @return Height in pixels
     */
    float getPixel_y() const
    {
        return pixel_y;
    }

    //=========================================================================
	// Shadow Bias
	//=========================================================================
	
    /**
     * @brief Set shadow bias
     * 
     * Bias to prevent shadow acne (z-fighting).
     * 
     * @param val Bias value
     */
    void setShadow_bias( float val )
    {
        shadow_bias = val;
    }
	
    /**
     * @brief Get shadow bias
     * @return Bias value
     */
    float getShadow_bias() const
    {
        return shadow_bias;
    }
	
    //=========================================================================
    // Public Data Members
    //=========================================================================
	
    /** @brief Shadow map matrix (world to shadow texture) */
    lsMatrix4x4 ShadowmapMatrix;
    
    /** @brief Ambient light color (RGBA) */
    lsVector4 ambient;
    
    /** @brief Diffuse light color (RGBA) */
    lsVector4 diffuse;
    
    /** @brief Specular light color (RGBA) */
    lsVector4 specular;
    
    /** @brief Light intensity multiplier */
    float intensity;
    
    /** @brief Spot light inner cone angle (degrees) */
    float spot_cutoff;
    
    /** @brief Spot light outer cone angle (degrees) */
    float spot_outer_cutoff;
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Light type (LSLT_*) */
    int light_type;

    /** @brief Shadow map texture ID */
    lsTextureID tex_shadowmap;
    
    /** @brief Shadow bias value */
    float shadow_bias;
    
    /** @brief Shadow map width */
    float pixel_x;
    
    /** @brief Shadow map height */
    float pixel_y;
    
    /** @brief Shadow map enabled flag */
    bool has_shadowmap;
    // Vulkan renders FBO depth maps Y-flipped vs GL (negative-height
    // viewport convention); the shadow bias matrix must flip V to match.
    // Set in Optimize() from the render backend type.
    bool m_flipShadowY = false;
};
