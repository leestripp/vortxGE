#pragma once

#include <iostream>
#include <string>

// Forward declarations to avoid circular dependencies
class lsStats;
class lsShaderManager;

class lsRender
{
public:
	lsRender();
	virtual ~lsRender(); // Virtual destructor is crucial for base classes
	
	// Core Initialization & Cleanup
	virtual void Init() = 0;
	// (No Cleanup() method here, destructor handles it, or specific per-API cleanup)
	
	// Generic Debugging & Error Checking
	virtual void CheckErrors(const std::string& name) = 0; // Renamed from CheckGLErrors
	
	// Generic Framebuffer Management
	virtual void StoreDefaultFramebuffer() = 0;
	virtual void RestoreDefaultFramebuffer() = 0;
	
	// Accessors for Managers (Generic to any Renderer)
	lsStats* getStats() const { return m_stats; }
	void setStats(lsStats* val) { m_stats = val; }
	
	lsShaderManager* getShaderManager() const { return m_shaderManager; }
	void setShaderManager(lsShaderManager* val) { m_shaderManager = val; }
	
protected:
	lsStats* m_stats;
	lsShaderManager* m_shaderManager;
};
