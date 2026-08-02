/**
 * @file lsGLTypes.h
 * @brief Renderer-agnostic API types
 *
 * Centralised typedefs for the renderer API surface defined on
 * `lsRender`. These types intentionally do **not** reference
 * `<GL/glew.h>` so the abstract base class can declare its full API
 * without dragging in OpenGL.
 *
 * Each concrete backend (`lsOpenGL`, a future `lsVulkan`, …) translates
 * these engine-level types into the matching backend-specific values at
 * the implementation boundary. Because the underlying numeric values
 * match the GL types (e.g. `GL_TEXTURE_2D == 0x0DE1 == lsEnum(...)`),
 * a future Vulkan backend can map them via lookup tables without
 * changing any caller code.
 *
 * @section handles GPU resource handles
 *
 * | lsType          | Underlying    | Maps to    |
 * |-----------------|---------------|------------|
 * | lsTextureID     | std::uint32_t | GLuint     |
 * | lsShaderID      | std::uint32_t | GLuint     |
 * | lsProgramID     | std::uint32_t | GLuint     |
 * | lsVAOID         | std::uint32_t | GLuint     |
 * | lsBufferID      | std::uint32_t | GLuint     |
 * | lsFBOID         | std::uint32_t | GLuint     |
 * | lsQueryID       | std::uint32_t | GLuint     |
 * | lsUniformID     | int           | GLint      |
 *
 * @section enum Enums and value types
 *
 * | lsType          | Underlying    | Maps to    |
 * |-----------------|---------------|------------|
 * | lsEnum          | unsigned int  | GLenum     |
 * | lsUInt          | unsigned int  | GLuint     |
 * | lsInt           | int           | GLint      |
 * | lsUByte         | unsigned char | GLubyte    |
 * | lsChar          | char          | GLchar     |
 * | lsFloat         | float         | GLfloat    |
 * | lsDouble        | double        | GLdouble   |
 * | lsBoolean       | unsigned char | GLboolean  |
 * | lsBitfield      | unsigned int  | GLbitfield |
 * | lsSizei         | int           | GLsizei    |
 * | lsSizeiptr      | std::intptr_t | GLsizeiptr |
 *
 * Concrete values (e.g. `GL_TEXTURE0`, `GL_RGBA`, `GL_TRIANGLES`) are
 * used at the call sites as plain integers — they will be `static_cast`
 * to `lsEnum` where needed. The intent is for those constants to
 * eventually be replaced by engine-level scoped enums (e.g.
 * `lsTextureTarget::Texture2D`); for the moment we accept that callers
 * may pass GL constants through.
 *
 * @see lsRender for the abstract API.
 * @see lsOpenGL for the OpenGL implementation.
 */

#pragma once

#include <cstdint>

//-------------------------------------------------------------------------
// Opaque GPU resource handles — never 0 except for "no resource"
//-------------------------------------------------------------------------

using lsTextureID  = std::uint32_t;   ///< Opaque texture object handle.
using lsShaderID   = std::uint32_t;   ///< Opaque shader object handle.
using lsProgramID  = std::uint32_t;   ///< Opaque shader-program handle.
using lsVAOID      = std::uint32_t;   ///< Opaque Vertex Array Object handle.
using lsBufferID   = std::uint32_t;   ///< Opaque Vertex/Index Buffer Object handle.
using lsFBOID      = std::uint32_t;   ///< Opaque Framebuffer Object handle.
using lsQueryID    = std::uint32_t;   ///< Opaque occlusion/query object handle.
using lsUniformID  = int;             ///< Opaque shader-uniform-location handle.

//-------------------------------------------------------------------------
// Enum and value types used in the lsRender API surface
//
// These mirror the GL numeric types exactly so that, at the OpenGL
// boundary, the values can be passed through with no conversion. A
// future Vulkan backend will translate them via lookup tables.
//-------------------------------------------------------------------------

using lsEnum       = unsigned int;    ///< Renderer enum value (GLenum).
using lsUInt       = unsigned int;    ///< Unsigned integer (GLuint/GLsizei where unsigned).
using lsInt        = int;             ///< Signed integer (GLint).
using lsUByte      = unsigned char;   ///< Unsigned byte (GLubyte/GLboolean).
using lsChar       = char;            ///< Plain char (GLchar).
using lsFloat      = float;           ///< Float (GLfloat/GLclampf).
using lsDouble     = double;          ///< Double (GLdouble).
using lsBoolean    = unsigned char;   ///< Boolean (GLboolean).
using lsBitfield   = unsigned int;    ///< Bitmask (GLbitfield).
using lsSizei      = int;             ///< Sized integer (GLsizei).
using lsSizeiptr   = std::intptr_t;   ///< Pointer-sized integer (GLsizeiptr).
