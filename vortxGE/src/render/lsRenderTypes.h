#pragma once

// Render Types - centralized enum for type safety
enum class RenderType {
	None = 0,
	OpenGL,
	Vulkan
};

// =========================================================================
// Pass-type constants (R6, 2026-08-14): moved out of the deleted legacy
// lsPass class. The scene graph now carries the pass type as a plain int.
// =========================================================================

/** @brief No operation pass */
const int LSPT_NONE             = 0;
/** @brief Direct rendering to screen (legacy) */
const int LSPT_DIRECTRENDER     = 1;
/** @brief Render to framebuffer object */
const int LSPT_FBO              = 2;
/** @brief Shadow map generation */
const int LSPT_SHADOWMAP        = 3;
/** @brief Occlusion query rendering (legacy) */
const int LSPT_OCCLUSION        = 4;
/** @brief Depth+normal prepass (pre-main-pass, for screen-space effects) */
const int LSPT_PREPASS          = 5;
