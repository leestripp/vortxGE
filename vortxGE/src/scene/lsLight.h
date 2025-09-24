#pragma once

// OpenGL
#include "../opengl/lsOpenGL.h"

// math
#include "../math/lsVector4.h"
#include "../math/lsMatrix4x4.h"

#include "lsCamera.h"

const int LSLT_NONE     = 0;
const int LSLT_POINT    = 1;
const int LSLT_SPOT     = 2;
const int LSLT_DIRECT   = 3;


class lsLight : public lsCamera
{
public:
    lsLight( lsNode* parent );
    virtual ~lsLight();

    // from lsCamera
    virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    // shadow map
    void buildShadowmap_matrix();
	
    // Save data
    virtual const string& toString();
	
    // setters getters
    void setLight_type( int val )
    {
        light_type = val;
    }

    int getLight_type() const
    {
        return light_type;
    }

    float getIntensity() const
    {
        return intensity;
    }

    void setIntensity(float val)
    {
        intensity = val;
    }


    // Shadowmap
    GLuint getTex_shadowmap() const
    {
        return tex_shadowmap;
    }

    void setTex_shadowmap(GLuint val)
    {
        tex_shadowmap = val;
    }

    bool getHas_shadowmap() const
    {
        return has_shadowmap;
    }

    void setHas_shadowmap(bool val)
    {
        has_shadowmap = val;
    }

    // Pixel sizes **********************************

    void setPixel_x( float val )
    {
        pixel_x = val;
    }

    float getPixel_x() const
    {
        return pixel_x;
    }

    void setPixel_y( float val )
    {
        pixel_y = val;
    }

    float getPixel_y() const
    {
        return pixel_y;
    }

    // Shadow Bias **********************************

    void setShadow_bias( float val )
    {
        shadow_bias = val;
    }
	
    float getShadow_bias() const
    {
        return shadow_bias;
    }
	
    // Data
    lsMatrix4x4 ShadowmapMatrix;
    lsVector4 ambient;
    lsVector4 diffuse;
    lsVector4 specular;
    float intensity;
    float spot_cutoff;		// angle of light cutoff
    float spot_outer_cutoff;
	
private:
    int light_type;

    GLuint tex_shadowmap;
    float shadow_bias;
    float pixel_x;
    float pixel_y;
    bool has_shadowmap;
};

