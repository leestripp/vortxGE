#ifndef LSPASS_OCCLUSION_H
#define LSPASS_OCCLUSION_H

#include "lsPass.h"

class lsPassManager;
class lsNode;
class lsRender;
class lsFBO_ColourDepth_8;


//! Occlusion Pass
/*!
 * Optimize objects depending on if they are hidden behind static walls etc...
 *
 * */
class lsPass_Occlusion : public lsPass
{
public:
	lsPass_Occlusion( int x, int y, int width, int height, lsRender *render );
	virtual ~lsPass_Occlusion();

	// from lsPass
    void Update();
    void Render();
	void Resize( int x, int y, int width, int height  );

    // Save
    virtual const string& toString();

private:
	lsFBO_ColourDepth_8 *fbo;

};

#endif // LSPASS_OCCLUSION_H
