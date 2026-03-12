/**
 * @file lsRender.h
 * @brief Abstract base class for rendering system
 * 
 * lsRender is the abstract base class for all rendering operations.
 * It provides a platform-agnostic interface for:
 * - OpenGL context management
 * - Viewport control
 * - Framebuffer management
 * - Shader management integration
 * - Performance statistics
 * 
 * @section implementation Platform-Specific Implementations
 * 
 * Concrete implementations exist for different platforms:
 * - lsRender_OpenGL: Standard OpenGL rendering
 * 
 * @section usage Usage
 * 
 * @code
 * lsRender* render = new lsRender_OpenGL();
 * render->Init();  // Initialize OpenGL context
 * 
 * // In render loop:
 * render->StoreDefaultFramebuffer();  // Before FBO rendering
 * // ... render to FBO ...
 * render->RestoreDefaultFramebuffer(); // After FBO rendering
 * render->CheckErrors("MainRender");
 * @endcode
 * 
 * @see lsRender_OpenGL for OpenGL implementation
 * @see lsStats for performance tracking
 * @see lsShaderManager for shader management
 */

#pragma once

#include <iostream>
#include <string>

// Types
#include "lsRenderTypes.h"

// Forward declarations
class lsStats;
class lsShaderManager;

using namespace std;

/**
 * @class lsRender
 * @brief Abstract base class for rendering system
 * 
 * This class defines the interface for all rendering operations.
 * Subclass this to add support for different rendering APIs
 * (OpenGL, Vulkan, DirectX, Metal, etc.)
 * 
 * @section stats Statistics
 * 
 * The render system integrates with lsStats to track:
 * - Frames per second (FPS)
 * - Draw call count
 * - Triangle count
 * - Texture memory usage
 * 
 * @note This class uses the Template Method pattern.
 *       Subclasses implement platform-specific methods.
 */
class lsRender
{
public:
    /**
     * @brief Default constructor
     * 
     * Initializes pointers to nullptr.
     * Subclass should call Init() after context creation.
     */
	lsRender();
	
    /**
     * @brief Virtual destructor
     * 
     * Cleans up render resources.
     * Must be virtual for proper cleanup of derived classes.
     */
	virtual ~lsRender();
	
    //=========================================================================
    // Abstract Methods (must be implemented by subclasses)
    //=========================================================================
	
    /**
     * @brief Initialize rendering system
     * 
     * Called after creating a valid graphics context.
     * Should set up default OpenGL states, create default
     * framebuffers, etc.
     * 
     * @pure
     */
	virtual void Init() = 0;
	
    /**
     * @brief Check for rendering errors
     * 
     * Queries the rendering API for errors and logs them.
     * Useful for debugging rendering issues.
     * 
     * @param name Label to identify where error check occurs
     * @pure
     */
	virtual void CheckErrors(const string& name) = 0;
	
    /**
     * @brief Store default framebuffer
     * 
     * Saves the current framebuffer binding so it can
     * be restored later. Used when rendering to FBOs.
     * 
     * @pure
     */
	virtual void StoreDefaultFramebuffer() = 0;
	
    /**
     * @brief Restore default framebuffer
     * 
     * Restores the previously stored default framebuffer.
     * Called after FBO rendering is complete.
     * 
     * @pure
     */
	virtual void RestoreDefaultFramebuffer() = 0;
	
    //=========================================================================
    // Accessors
    //=========================================================================
	
    /**
     * @brief Get performance statistics
     * @return Pointer to stats object
     */
	lsStats* getStats() const { return m_stats; }
	
    /**
     * @brief Set performance statistics
     * @param val Pointer to stats object
     */
	void setStats(lsStats* val) { m_stats = val; }
	
    /**
     * @brief Get shader manager
     * @return Pointer to shader manager
     */
	lsShaderManager* getShaderManager() const { return m_shaderManager; }
	
    /**
     * @brief Set shader manager
     * @param val Pointer to shader manager
     */
	void setShaderManager(lsShaderManager* val) { m_shaderManager = val; }
	
    /**
     * @brief Set render type
     * @param val Render type (OPENGL, VULKAN, etc.)
     */
	void setType( RenderType val ) { m_type = val; }
	
    /**
     * @brief Get render type
     * @return Current render type
     */
	RenderType getType() const { return m_type; }
	
protected:
    /**
     * @brief Current render type
     */
	RenderType m_type;
	
    /**
     * @brief Performance statistics tracker
     */
	lsStats* m_stats;
	
    /**
     * @brief Shader manager for shader operations
     */
	lsShaderManager* m_shaderManager;
};
