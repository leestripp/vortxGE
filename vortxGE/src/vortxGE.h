/**
 * @file vortxGE.h
 * @brief Main engine core header for vortxGE 3D Graphics Engine
 * 
 * This is the main entry point header for the vortxGE graphics engine.
 * Create an instance of vortxGE in your application to access all engine
 * subsystems including rendering, asset management, shaders, physics, and controls.
 * 
 * @mainpage vortxGE 3D Graphics Engine
 * 
 * @section usage Usage
 * 
 * Typical initialization and main loop:
 * @code
 * vortxGE* engine = new vortxGE();
 * 
 * // After creating a render context (OpenGL)
 * engine->Init();
 * 
 * // Main loop
 * while (!quit) {
 *     double delta = get_delta_time();
 *     engine->Update(delta);
 *     engine->Render();
 * }
 * 
 * engine->Cleanup();
 * delete engine;
 * @endcode
 * 
 * @section subsystems Engine Subsystems
 * 
 * - lsRender: OpenGL rendering system
 * - lsShaderManager: Shader compilation and management
 * - lsAssetManager: Resource loading (meshes, textures, physics shapes)
 * - lsControlManager: User input (viewer, menus, gameplay)
 * - lsOpenAL: 3D audio system
 * - lsScene/lsSceneManager: Scene graph and object management
 * 
 * @author vortxGE Team
 * @date 2024
 */

#pragma once

#include <iostream>
#include <string>

// Forward declarations to avoid circular dependencies
class lsStats;
class lsRender;
class lsShaderManager;
class lsControlManager;
class lsAssetManager;
class lsOpenAL;

using namespace std;

/**
 * @class vortxGE
 * @brief Main engine core class - entry point for all engine functionality
 * 
 * This is the primary interface to the vortxGE graphics engine. It manages
 * all core subsystems and provides access to them through getter methods.
 * 
 * @section initialization Initialization
 * 
 * After creating a render context (GLFW, GTKmm, etc.), call Init() to
 * initialize all engine subsystems. The engine requires a working OpenGL
 * context before calling Init().
 * 
 * @section mainloop Main Loop
 * 
 * In your application loop, call Update() and Render() in sequence.
 * Update() processes logic, animations, physics, and input.
 * Render() executes the rendering pipeline.
 * 
 * @subsection delta_time Delta Time
 * 
 * Pass the time elapsed since the last frame (in seconds) to Update().
 * This is used for frame-rate independent animation and physics.
 * 
 * @note Not thread-safe. Call from the main thread only.
 * @see lsScene for scene management
 * @see lsRender for rendering system
 */

class vortxGE
{
public:
    /**
     * @brief Default constructor
     * 
     * Creates a new vortxGE engine instance. All subsystems are created
     * but not initialized until Init() is called.
     * 
     * @post All subsystem pointers are initialized to nullptr
     */
	vortxGE();
	
    /**
     * @brief Virtual destructor
     * 
     * Cleans up all engine subsystems. Calls Cleanup() if not already called.
     */
	virtual ~vortxGE();
	
    /**
     * @brief Initialize all engine subsystems
     * 
     * Must be called after creating a valid OpenGL render context.
     * Initializes the following subsystems in order:
     * 1. lsRender - Graphics rendering
     * 2. lsShaderManager - Shader compilation
     * 3. lsStats - Performance statistics
     * 4. lsAssetManager - Asset loading
     * 5. lsOpenAL - Audio system
     * 6. lsControlManager - Input handling
     * 
     * @return 0 on success, non-zero on failure
     * @pre Valid OpenGL context must exist
     * @post All subsystems are initialized and ready for use
     */
	int Init();
	
    /**
     * @brief Cleanup and shutdown all engine subsystems
     * 
     * Releases all resources allocated by engine subsystems.
     * Called automatically by destructor if not called explicitly.
     * 
     * @post All subsystems are cleaned up
     */
	void Cleanup();
	
    /**
     * @brief Handle window resize events
     * 
     * Notifies all engine subsystems of a window size change.
     * Updates viewport dimensions and internal buffers as needed.
     * 
     * @param x Left coordinate of the window
     * @param y Top coordinate of the window
     * @param width New width of the window in pixels
     * @param height New height of the window in pixels
     */
	void Resize( int x, int y, int width, int height );
	
	// Main loop
	
    /**
     * @brief Update engine state for one frame
     * 
     * Processes all time-dependent updates including:
     * - Animation playback
     * - Physics simulation
     * - Input processing
     * - Scene graph updates
     * 
     * @param delta Time elapsed since last frame in seconds
     * @note Use consistent delta times for smooth animation
     */
	void Update( double delta );
	
    /**
     * @brief Render the current frame
     * 
     * Executes the rendering pipeline to draw the scene.
     * Should be called after Update() in the main loop.
     * 
     * @note Depends on Update() being called first
     */
	void Render();
	
    /**
     * @brief Get the control manager for input handling
     * 
     * Access the control manager to handle user input,
     * including keyboard, mouse, and gamepad events.
     * 
     * @return Pointer to the lsControlManager instance
     * @post Returns valid pointer after Init()
     * @see lsControlManager
     */
	lsControlManager* getControlManager() const
	{
		return controlmanager;
	}

    /**
     * @brief Get the shader manager for shader operations
     * 
     * Access the shader manager to load, compile, and manage
     * GLSL shader programs.
     * 
     * @return Pointer to the lsShaderManager instance
     * @post Returns valid pointer after Init()
     * @see lsShaderManager
     */
	lsShaderManager* getShaderManager() const
	{
		return shadermanager;
	}

    /**
     * @brief Get the asset manager for resource loading
     * 
     * Access the asset manager to load meshes, textures,
     * and physics shapes from disk.
     * 
     * @return Pointer to the lsAssetManager instance
     * @post Returns valid pointer after Init()
     * @see lsAssetManager
     */
	lsAssetManager* getAssetManager() const
	{
		return assetmanager;
	}
	
    /**
     * @brief Get the render system
     * 
     * Access the rendering system for OpenGL operations,
     * viewport management, and draw calls.
     * 
     * @return Pointer to the lsRender instance
     * @post Returns valid pointer after Init()
     * @see lsRender
     */
	lsRender* getRender() const
	{
		return render;
	}
	
    /**
     * @brief Get the audio system
     * 
     * Access the OpenAL audio system for 3D sound
     * and spatial audio.
     * 
     * @return Pointer to the lsOpenAL instance
     * @post Returns valid pointer after Init()
     * @see lsOpenAL
     */
	lsOpenAL* getOpenAL() const
	{
		return openal;
	}
	
private:
	// Managers - Core engine subsystems
	
    /** @brief Rendering system - handles all OpenGL operations */
	lsRender            *render;
	
    /** @brief Audio system - handles OpenAL sound */
	lsOpenAL            *openal;
	
    /** @brief Shader manager - handles shader compilation and binding */
	lsShaderManager     *shadermanager;
	
    /** @brief Asset manager - handles resource loading */
	lsAssetManager      *assetmanager;
	
    /** @brief Control manager - handles user input */
	lsControlManager    *controlmanager;
	
    /** @brief Statistics tracker - FPS and performance metrics */
	lsStats             *stats;
};
