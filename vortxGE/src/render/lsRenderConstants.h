/**
 * @file lsRenderConstants.h
 * @brief Engine-level renderer constants (LS_*)
 *
 * These constants are the public, backend-agnostic names for the enum values
 * that renderers interpret. Each LS_* is a thin alias for a backend-specific
 * constant (currently GL_*) — the OpenGL implementation passes them
 * through unchanged (values are GL-identical); the Vulkan backend maps
 * them to Vk* enums at its boundary (lsVulkanTypes.h).
 *
 * Anywhere outside the renderer implementation (i.e. anywhere except
 * render/lsOpenGL.{h,cpp}) should refer to LS_*, never GL_*.
 *
 * Only render/lsOpenGL.cpp needs <GL/glew.h>. Non-renderer files include
 * this header instead, which exposes LS_* as numeric literals (identical
 * to the GL values) without any GL/Vulkan header dependency.
 */

#ifndef LS_RENDER_CONSTANTS_H
#define LS_RENDER_CONSTANTS_H

// NOTE: LS_* values are numeric literals identical to the GL enum values
// (extracted from the system GL headers; the original GL_* name is kept in
// a trailing comment on each line). This keeps the header — and therefore
// lsRender.h and every engine call site — free of any GL/Vulkan headers.
// The OpenGL backend still includes <GL/glew.h> itself in lsOpenGL.cpp and
// relies on these values matching GL's (enforced by extraction, not faith).
// The Vulkan backend maps these values to Vk* enums in lsVulkanTypes.h.

// Texture binding targets
#define LS_TEXTURE_2D  0x0DE1  /* GL_TEXTURE_2D */

// Texture parameters
#define LS_TEXTURE_WRAP_S  0x2802  /* GL_TEXTURE_WRAP_S */
#define LS_TEXTURE_WRAP_T  0x2803  /* GL_TEXTURE_WRAP_T */
#define LS_TEXTURE_BORDER_COLOR  0x1004  /* GL_TEXTURE_BORDER_COLOR */
#define LS_TEXTURE_MIN_FILTER  0x2801  /* GL_TEXTURE_MIN_FILTER */
#define LS_TEXTURE_MAG_FILTER  0x2800  /* GL_TEXTURE_MAG_FILTER */
#define LS_UNPACK_ALIGNMENT  0x0CF5  /* GL_UNPACK_ALIGNMENT */

// Texture filtering modes
#define LS_LINEAR  0x2601  /* GL_LINEAR */
#define LS_NEAREST  0x2600  /* GL_NEAREST */
#define LS_LINEAR_MIPMAP_LINEAR  0x2703  /* GL_LINEAR_MIPMAP_LINEAR */

// Texture wrap modes
#define LS_REPEAT  0x2901  /* GL_REPEAT */
#define LS_CLAMP_TO_EDGE  0x812F  /* GL_CLAMP_TO_EDGE */
#define LS_CLAMP_TO_BORDER  0x812D  /* GL_CLAMP_TO_BORDER */

// Texture pixel formats
#define LS_RED  0x1903  /* GL_RED */
#define LS_RGB  0x1907  /* GL_RGB */
#define LS_RGBA  0x1908  /* GL_RGBA */
#define LS_BGR  0x80E0  /* GL_BGR */
#define LS_BGRA  0x80E1  /* GL_BGRA */
#define LS_ALPHA  0x1906  /* GL_ALPHA */
#define LS_LUMINANCE  0x1909  /* GL_LUMINANCE */

// Sized internal formats
#define LS_R8  0x8229  /* GL_R8 */
#define LS_RGB16  0x8054  /* GL_RGB16 */
#define LS_RGBA16  0x805B  /* GL_RGBA16 */
#define LS_RGB32F  0x8815  /* GL_RGB32F */
#define LS_RGBA32F  0x8814  /* GL_RGBA32F */
#define LS_SRGB  0x8C40  /* GL_SRGB */
#define LS_SRGB_ALPHA  0x8C42  /* GL_SRGB_ALPHA */
#define LS_SRGB8  0x8C41  /* GL_SRGB8 */
#define LS_SRGB8_ALPHA8  0x8C43  /* GL_SRGB8_ALPHA8 */

// Packed formats
#define LS_DEPTH_COMPONENT  0x1902  /* GL_DEPTH_COMPONENT */

// Pixel data types
#define LS_FLOAT  0x1406  /* GL_FLOAT */
#define LS_INT  0x1404  /* GL_INT */
#define LS_UNSIGNED_BYTE  0x1401  /* GL_UNSIGNED_BYTE */
#define LS_UNSIGNED_SHORT  0x1403  /* GL_UNSIGNED_SHORT */
#define LS_UNSIGNED_INT  0x1405  /* GL_UNSIGNED_INT */

// Buffer bindings
#define LS_ARRAY_BUFFER  0x8892  /* GL_ARRAY_BUFFER */
#define LS_ELEMENT_ARRAY_BUFFER  0x8893  /* GL_ELEMENT_ARRAY_BUFFER */

// Buffer usage hints
#define LS_STATIC_DRAW  0x88E4  /* GL_STATIC_DRAW */
#define LS_DYNAMIC_DRAW  0x88E8  /* GL_DYNAMIC_DRAW */
#define LS_STREAM_DRAW  0x88E0  /* GL_STREAM_DRAW */

// Shaders / programs
#define LS_VERTEX_SHADER  0x8B31  /* GL_VERTEX_SHADER */
#define LS_FRAGMENT_SHADER  0x8B30  /* GL_FRAGMENT_SHADER */
#define LS_INFO_LOG_LENGTH  0x8B84  /* GL_INFO_LOG_LENGTH */

// Framebuffers
#define LS_FRAMEBUFFER  0x8D40  /* GL_FRAMEBUFFER */
#define LS_FRAMEBUFFER_COMPLETE  0x8CD5  /* GL_FRAMEBUFFER_COMPLETE */
#define LS_FRAMEBUFFER_BINDING  0x8CA6  /* GL_FRAMEBUFFER_BINDING */
#define LS_COLOR_ATTACHMENT0  0x8CE0  /* GL_COLOR_ATTACHMENT0 */
#define LS_COLOR_ATTACHMENT1  0x8CE1  /* GL_COLOR_ATTACHMENT1 */
#define LS_DEPTH_ATTACHMENT  0x8D00  /* GL_DEPTH_ATTACHMENT */

// Draw buffers
#define LS_NONE  0  /* GL_NONE */

// Clear bits
#define LS_COLOR_BUFFER_BIT  0x00004000  /* GL_COLOR_BUFFER_BIT */
#define LS_DEPTH_BUFFER_BIT  0x00000100  /* GL_DEPTH_BUFFER_BIT */
#define LS_STENCIL_BUFFER_BIT  0x00000400  /* GL_STENCIL_BUFFER_BIT */

// Face culling
#define LS_FRONT  0x0404  /* GL_FRONT */
#define LS_BACK  0x0405  /* GL_BACK */
#define LS_FRONT_AND_BACK  0x0408  /* GL_FRONT_AND_BACK */

// Depth testing
#define LS_NEVER  0x0200  /* GL_NEVER */
#define LS_LESS  0x0201  /* GL_LESS */
#define LS_EQUAL  0x0202  /* GL_EQUAL */
#define LS_LEQUAL  0x0203  /* GL_LEQUAL */
#define LS_GREATER  0x0204  /* GL_GREATER */
#define LS_GEQUAL  0x0206  /* GL_GEQUAL */
#define LS_ALWAYS  0x0207  /* GL_ALWAYS */

// Capabilities
#define LS_CULL_FACE  0x0B44  /* GL_CULL_FACE */
#define LS_DEPTH_TEST  0x0B71  /* GL_DEPTH_TEST */
#define LS_BLEND  0x0BE2  /* GL_BLEND */
#define LS_SCISSOR_TEST  0x0C11  /* GL_SCISSOR_TEST */
#define LS_POLYGON_OFFSET_FILL  0x8037  /* GL_POLYGON_OFFSET_FILL */
#define LS_TEXTURE0  0x84C0  /* GL_TEXTURE0 */
#define LS_TEXTURE1  0x84C1  /* GL_TEXTURE1 */
#define LS_TEXTURE2  0x84C2  /* GL_TEXTURE2 */
#define LS_TEXTURE3  0x84C3  /* GL_TEXTURE3 */
#define LS_TEXTURE4  0x84C4  /* GL_TEXTURE4 */
#define LS_TEXTURE5  0x84C5  /* GL_TEXTURE5 */
#define LS_TEXTURE6  0x84C6  /* GL_TEXTURE6 */
#define LS_TEXTURE7  0x84C7  /* GL_TEXTURE7 */

// Blending factors
#define LS_SRC_ALPHA  0x0302  /* GL_SRC_ALPHA */
#define LS_ONE_MINUS_SRC_ALPHA  0x0303  /* GL_ONE_MINUS_SRC_ALPHA */

// Polygons
#define LS_TRIANGLES  0x0004  /* GL_TRIANGLES */
#define LS_LINES  0x0001  /* GL_LINES */
#define LS_LINE  0x1B01  /* GL_LINE */

// Bool
#define LS_FALSE  0  /* GL_FALSE */
#define LS_TRUE  1  /* GL_TRUE */

// Occlusion queries
#define LS_SAMPLES_PASSED  0x8914  /* GL_SAMPLES_PASSED */
#define LS_QUERY_RESULT  0x8866  /* GL_QUERY_RESULT */

// Polygonal shading modes (legacy; rarely used but kept for completeness)
#define LS_FLAT  0x1D00  /* GL_FLAT */
#define LS_SMOOTH  0x1D01  /* GL_SMOOTH */
#define LS_LIGHTING  0x0B50  /* GL_LIGHTING */  // legacy fixed-function; only used by commented-out lsPlane::drawGuide()

// State queries
#define LS_VIEWPORT  0x0BA2  /* GL_VIEWPORT */

// String queries (used by lsOpenGL::GetInfo)
#define LS_VERSION  0x1F02  /* GL_VERSION */
#define LS_VENDOR  0x1F00  /* GL_VENDOR */
#define LS_RENDERER  0x1F01  /* GL_RENDERER */
#define LS_SHADING_LANGUAGE_VERSION  0x8B8C  /* GL_SHADING_LANGUAGE_VERSION */

// Error codes (used by lsOpenGL::CheckGLErrors)
#define LS_NO_ERROR  0  /* GL_NO_ERROR */
#define LS_INVALID_ENUM  0x0500  /* GL_INVALID_ENUM */
#define LS_INVALID_VALUE  0x0501  /* GL_INVALID_VALUE */
#define LS_INVALID_OPERATION  0x0502  /* GL_INVALID_OPERATION */
#define LS_INVALID_FRAMEBUFFER_OPERATION  0x0506  /* GL_INVALID_FRAMEBUFFER_OPERATION */
#define LS_INVALID_INDEX  0xFFFFFFFF  /* GL_INVALID_INDEX */

#endif // LS_RENDER_CONSTANTS_H
// Additional constants needed by the Vulkan backend mappings
// (lsVulkanTypes.h) — same literal-value convention as above.
#define LS_ZERO  0  /* GL_ZERO */
#define LS_ONE  1  /* GL_ONE */
#define LS_SRC_COLOR  0x0300  /* GL_SRC_COLOR */
#define LS_ONE_MINUS_SRC_COLOR  0x0301  /* GL_ONE_MINUS_SRC_COLOR */
#define LS_DST_COLOR  0x0306  /* GL_DST_COLOR */
#define LS_ONE_MINUS_DST_COLOR  0x0307  /* GL_ONE_MINUS_DST_COLOR */
#define LS_DST_ALPHA  0x0304  /* GL_DST_ALPHA */
#define LS_ONE_MINUS_DST_ALPHA  0x0305  /* GL_ONE_MINUS_DST_ALPHA */
#define LS_NOTEQUAL  0x0205  /* GL_NOTEQUAL */
#define LS_TRIANGLE_STRIP  0x0005  /* GL_TRIANGLE_STRIP */
#define LS_LINE_STRIP  0x0003  /* GL_LINE_STRIP */
#define LS_POINTS  0x0000  /* GL_POINTS */
#define LS_COMPILE_STATUS  0x8B81  /* GL_COMPILE_STATUS */
#define LS_LINK_STATUS  0x8B82  /* GL_LINK_STATUS */
#define LS_MIRRORED_REPEAT  0x8370  /* GL_MIRRORED_REPEAT */
