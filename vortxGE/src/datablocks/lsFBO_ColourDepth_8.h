#ifndef LSFBO_COLOURDEPTH_8_H
#define LSFBO_COLOURDEPTH_8_H

#include "../render/lsOpenGL.h"

#include "lsFBO.h"


//! OpenGL Frame Buffer Object
/**
 If subclassing this please remember to Store and restore the default IOS framebuffer\n
 IOS uses its own framebuffer object to draw into and at runtime we dont know what it is, so store the current one and then restore it before and afer you Bind to your buffer.
 */
class lsFBO_ColourDepth_8 : public lsFBO
{
public:
	lsFBO_ColourDepth_8( double width, double height, lsOpenGL *opengl );
	virtual ~lsFBO_ColourDepth_8();

    // Form lsFBO
	void Enable();
	void ClearBuffers();

    // Data

	GLuint getTexture()
	{
		return texture;
	}

	GLuint getDepthTexture()
	{
		return depth_texture;
	}


private:
	GLuint texture;
	GLuint depth_texture;

};

#endif // LSFBO_COLOURDEPTH_8_H
