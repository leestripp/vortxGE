#ifndef vortxGE_lsPass_DR_h
#define vortxGE_lsPass_DR_h

#include "lsPass.h"


/** \brief Basic pass that uses the default system Framebuffer
 *
 * You bust setup a default GL framebuffer with at least Colour and Depth buffers.
 */

class lsPass_DR : public lsPass
{
public:
    lsPass_DR( lsRender* render );
    virtual ~lsPass_DR();

    void Update();
    void Render();
	void Resize( int x, int y, int width, int height  );

    // Save
    virtual const string& toString();

private:

};


#endif
