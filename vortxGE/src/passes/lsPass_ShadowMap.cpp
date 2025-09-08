#include "../vortx_debug.h"
#include "../opengl/lsOpenGL.h"
#include "../datablocks/lsStats.h"
#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"
#include "lsPass_ShadowMap.h"

lsPass_ShadowMap::lsPass_ShadowMap( int x, int y, int width, int height, lsOpenGL *opengl ) : lsPass(opengl)
{
    setPass_type( LSPT_SHADOWMAP );
    setName("Pass : Shadow Map");

    fbo = new lsFBO_Depth( width, height, opengl );
    if (!fbo)
    {
        cout << "lsPass_ShadowMap::lsPass_ShadowMap: ERROR - could not create lsFBO_Depth instance" << endl;
    }
    Resize(x, y, width, height);
    setSize(lsVector2(width, height));
}

lsPass_ShadowMap::~lsPass_ShadowMap()
{
    if (fbo) delete fbo;
}

void lsPass_ShadowMap::Update()
{
    if (getScene()) getScene()->InitDrawlist();

    if (getScene() && getCull_camera())
    {
        // We use the light as the cull camera for the shadow pass
        if (getOgl()->getStats()) getOgl()->getStats()->Optimize_Start();
        getScene()->OptimizeThread(getCull_camera(), this, getScene(), getOgl());
        if (getOgl()->getStats()) getOgl()->getStats()->Optimize_Stop();
    }
}

void lsPass_ShadowMap::Render()
{
    if (fbo && getLight())
    {
        getOgl()->StoreFBO();
        fbo->Enable();

        glViewport(getPosition().x, getPosition().y, getSize().x, getSize().y);
        
        glEnable(GL_DEPTH_TEST);
        
        // Use polygon offset to combat shadow acne. This is more robust
        // than just shader-side bias or front-face culling.
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(4.0f, 4.0f);
        
        fbo->ClearBuffers();
        
        // Render scene from light's perspective
        if (getScene()) getScene()->Draw(getLight(), this, NULL, getOgl());

        // Restore OpenGL state
        glDisable(GL_POLYGON_OFFSET_FILL);
        
        getOgl()->RestoreFBO();
    }
}

void lsPass_ShadowMap::Resize(int x, int y, int width, int height)
{
    // This pass might have a fixed size FBO, but we can still set viewport info
    setPosition(lsVector2(x, y));
    // The size is determined by FBO creation, but we can update ratio for camera
    if (width > 0 && height > 0)
    {
        setVratio((float)width / (float)height);
    }

    if (getCamera())
    {
        getCamera()->Set_ViewRatio(getVratio());
    }
    if (getCull_camera())
    {
        getCull_camera()->Set_ViewRatio(getVratio());
    }
}

const string& lsPass_ShadowMap::toString()
{
    // TODO: Implement saving if needed
    setSave_string("pass_shadowmap");
    return getSave_string();
}
