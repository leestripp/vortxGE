#ifndef vortxGE_lsComposite_h
#define vortxGE_lsComposite_h

#include "lsControl.h"

class lsSceneManager;
class lsPassManager;
class lsAssetManager;
class lsOpenGL;

//! Layered renders
/**
 * Use textures (FBO) from passes to build final output. But really its up to you.
 *
 * Contains a Pass Manager, Scene Manager and Asset Manager.
 *
 * */

class lsComposite : public lsControl
{
public:
    lsComposite( lsOpenGL *opengl, lsControl* p );
    virtual ~lsComposite();

    int Init();

    void Update( double delta );
    void Render();
	void Resize( int x, int y, int width, int height  );

    // get set
    lsSceneManager* getSceneManager() const
    {
        return scenemanager;
    }

    lsPassManager* getPassManager() const
    {
        return passmanager;
    }

    lsAssetManager* getAssetManager() const
    {
        return assetmanager;
    }


private:
    lsSceneManager      *scenemanager;
    lsPassManager       *passmanager;
    lsAssetManager      *assetmanager;

};


#endif
