#ifndef vortxGE_IOS_lsViewer_h
#define vortxGE_IOS_lsViewer_h

#include "lsControl.h"

class vortxGE;
class lsSceneManager;
class lsPassManager;
class lsAssetManager;
class lsOpenGL;

//! Final output
/*!
 * Use this to combine everything for output, also good for simple scene renders.
 *
 * Creates a Scene Manager, Pass Manager and Asset Manager.
 *
 * */

class lsViewer : public lsControl
{
public:
    lsViewer( lsOpenGL *opengl, lsControl* p );
    virtual ~lsViewer();

    int Init();

    void Update( double delta );
    void Render();
	void Resize( int x, int y, int width, int height  );

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
