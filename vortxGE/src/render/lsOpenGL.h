#pragma once

#include <iostream>
#include <string>

// openGL
#include <GL/glew.h>

// Base class
#include "lsRender.h"

class lsStats;
class lsShaderManager;

using namespace std;

class lsOpenGL : public lsRender
{
public:
	lsOpenGL();
	virtual ~lsOpenGL();
	
	// Implement pure virtual methods from lsRender:
	virtual void Init() override;
	// Errors
	virtual void CheckErrors( const string &name ) override;
	// Framebuffer Management
	virtual void StoreDefaultFramebuffer() override;
	virtual void RestoreDefaultFramebuffer() override;
	
	// OpenGL-specific methods:
	void glInfo();
	void rendererChanged();
	
private:
	// Store FBO
	GLint m_defaultFBO;
};
