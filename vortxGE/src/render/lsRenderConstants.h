/**
 * @file lsRenderConstants.h
 * @brief Engine-level renderer constants (LS_*)
 *
 * These constants are the public, backend-agnostic names for the enum values
 * that renderers interpret. Each LS_* is a thin alias for a backend-specific
 * constant (currently GL_*) — the current OpenGL implementation passes them
 * through to the real GL_* values. A future Vulkan/Metal backend would
 * redefine this header so its LS_* values map to the corresponding
 * VkPipelineStageFlagBits / MTLBlitOption etc.
 *
 * Anywhere outside the renderer implementation (i.e. anywhere except
 * render/lsOpenGL.{h,cpp}) should refer to LS_*, never GL_*.
 *
 * Only render/lsOpenGL.cpp needs <GL/glew.h>. Non-renderer files include
 * this header instead, which exposes LS_* without the GLEW dependency.
 */

#ifndef LS_RENDER_CONSTANTS_H
#define LS_RENDER_CONSTANTS_H

#include <GL/glew.h>

// Texture binding targets
#define LS_TEXTURE_2D                  GL_TEXTURE_2D

// Texture parameters
#define LS_TEXTURE_WRAP_S              GL_TEXTURE_WRAP_S
#define LS_TEXTURE_WRAP_T              GL_TEXTURE_WRAP_T
#define LS_TEXTURE_BORDER_COLOR        GL_TEXTURE_BORDER_COLOR
#define LS_TEXTURE_MIN_FILTER          GL_TEXTURE_MIN_FILTER
#define LS_TEXTURE_MAG_FILTER          GL_TEXTURE_MAG_FILTER
#define LS_UNPACK_ALIGNMENT            GL_UNPACK_ALIGNMENT

// Texture filtering modes
#define LS_LINEAR                      GL_LINEAR
#define LS_NEAREST                     GL_NEAREST
#define LS_LINEAR_MIPMAP_LINEAR        GL_LINEAR_MIPMAP_LINEAR

// Texture wrap modes
#define LS_REPEAT                      GL_REPEAT
#define LS_CLAMP_TO_EDGE               GL_CLAMP_TO_EDGE
#define LS_CLAMP_TO_BORDER             GL_CLAMP_TO_BORDER

// Texture pixel formats
#define LS_RED                         GL_RED
#define LS_RGB                         GL_RGB
#define LS_RGBA                        GL_RGBA
#define LS_BGR                         GL_BGR
#define LS_BGRA                        GL_BGRA
#define LS_ALPHA                       GL_ALPHA
#define LS_LUMINANCE                   GL_LUMINANCE

// Sized internal formats
#define LS_R8                          GL_R8
#define LS_RGB16                       GL_RGB16
#define LS_RGBA16                      GL_RGBA16
#define LS_RGB32F                      GL_RGB32F
#define LS_RGBA32F                     GL_RGBA32F
#define LS_SRGB                        GL_SRGB
#define LS_SRGB_ALPHA                  GL_SRGB_ALPHA
#define LS_SRGB8                       GL_SRGB8
#define LS_SRGB8_ALPHA8                GL_SRGB8_ALPHA8

// Packed formats
#define LS_DEPTH_COMPONENT             GL_DEPTH_COMPONENT

// Pixel data types
#define LS_FLOAT                       GL_FLOAT
#define LS_INT                         GL_INT
#define LS_UNSIGNED_BYTE               GL_UNSIGNED_BYTE
#define LS_UNSIGNED_SHORT              GL_UNSIGNED_SHORT
#define LS_UNSIGNED_INT                GL_UNSIGNED_INT

// Buffer bindings
#define LS_ARRAY_BUFFER                GL_ARRAY_BUFFER
#define LS_ELEMENT_ARRAY_BUFFER        GL_ELEMENT_ARRAY_BUFFER

// Buffer usage hints
#define LS_STATIC_DRAW                 GL_STATIC_DRAW
#define LS_DYNAMIC_DRAW                GL_DYNAMIC_DRAW
#define LS_STREAM_DRAW                 GL_STREAM_DRAW

// Shaders / programs
#define LS_VERTEX_SHADER               GL_VERTEX_SHADER
#define LS_FRAGMENT_SHADER             GL_FRAGMENT_SHADER
#define LS_INFO_LOG_LENGTH             GL_INFO_LOG_LENGTH

// Framebuffers
#define LS_FRAMEBUFFER                 GL_FRAMEBUFFER
#define LS_FRAMEBUFFER_COMPLETE        GL_FRAMEBUFFER_COMPLETE
#define LS_FRAMEBUFFER_BINDING         GL_FRAMEBUFFER_BINDING
#define LS_COLOR_ATTACHMENT0           GL_COLOR_ATTACHMENT0
#define LS_COLOR_ATTACHMENT1           GL_COLOR_ATTACHMENT1
#define LS_DEPTH_ATTACHMENT            GL_DEPTH_ATTACHMENT

// Draw buffers
#define LS_NONE                        GL_NONE

// Clear bits
#define LS_COLOR_BUFFER_BIT            GL_COLOR_BUFFER_BIT
#define LS_DEPTH_BUFFER_BIT            GL_DEPTH_BUFFER_BIT
#define LS_STENCIL_BUFFER_BIT          GL_STENCIL_BUFFER_BIT

// Face culling
#define LS_FRONT                       GL_FRONT
#define LS_BACK                        GL_BACK
#define LS_FRONT_AND_BACK              GL_FRONT_AND_BACK

// Depth testing
#define LS_NEVER                       GL_NEVER
#define LS_LESS                        GL_LESS
#define LS_EQUAL                       GL_EQUAL
#define LS_LEQUAL                      GL_LEQUAL
#define LS_GREATER                     GL_GREATER
#define LS_GEQUAL                      GL_GEQUAL
#define LS_ALWAYS                      GL_ALWAYS

// Capabilities
#define LS_CULL_FACE                   GL_CULL_FACE
#define LS_DEPTH_TEST                  GL_DEPTH_TEST
#define LS_BLEND                       GL_BLEND
#define LS_SCISSOR_TEST                GL_SCISSOR_TEST
#define LS_POLYGON_OFFSET_FILL         GL_POLYGON_OFFSET_FILL
#define LS_TEXTURE0                    GL_TEXTURE0
#define LS_TEXTURE1                    GL_TEXTURE1
#define LS_TEXTURE2                    GL_TEXTURE2
#define LS_TEXTURE3                    GL_TEXTURE3
#define LS_TEXTURE4                    GL_TEXTURE4
#define LS_TEXTURE5                    GL_TEXTURE5
#define LS_TEXTURE6                    GL_TEXTURE6
#define LS_TEXTURE7                    GL_TEXTURE7

// Blending factors
#define LS_SRC_ALPHA                   GL_SRC_ALPHA
#define LS_ONE_MINUS_SRC_ALPHA         GL_ONE_MINUS_SRC_ALPHA

// Polygons
#define LS_TRIANGLES                   GL_TRIANGLES
#define LS_LINES                       GL_LINES
#define LS_LINE                        GL_LINE

// Bool
#define LS_FALSE                       GL_FALSE
#define LS_TRUE                        GL_TRUE

// Occlusion queries
#define LS_SAMPLES_PASSED              GL_SAMPLES_PASSED
#define LS_QUERY_RESULT                GL_QUERY_RESULT

// Polygonal shading modes (legacy; rarely used but kept for completeness)
#define LS_FLAT                        GL_FLAT
#define LS_SMOOTH                      GL_SMOOTH
#define LS_LIGHTING                    GL_LIGHTING  // legacy fixed-function; only used by commented-out lsPlane::drawGuide()

// State queries
#define LS_VIEWPORT                    GL_VIEWPORT

// String queries (used by lsOpenGL::GetInfo)
#define LS_VERSION                     GL_VERSION
#define LS_VENDOR                      GL_VENDOR
#define LS_RENDERER                    GL_RENDERER
#define LS_SHADING_LANGUAGE_VERSION    GL_SHADING_LANGUAGE_VERSION

// Error codes (used by lsOpenGL::CheckGLErrors)
#define LS_NO_ERROR                    GL_NO_ERROR
#define LS_INVALID_ENUM                GL_INVALID_ENUM
#define LS_INVALID_VALUE               GL_INVALID_VALUE
#define LS_INVALID_OPERATION           GL_INVALID_OPERATION
#define LS_INVALID_FRAMEBUFFER_OPERATION GL_INVALID_FRAMEBUFFER_OPERATION
#define LS_INVALID_INDEX               GL_INVALID_INDEX

#endif // LS_RENDER_CONSTANTS_H
