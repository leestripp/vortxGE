/**
 * @file lsScreenshot.h
 * @brief Shared framebuffer-screenshot file writer for render backends.
 *
 * Backends (lsOpenGL, lsVulkan) read back their framebuffer themselves,
 * then hand the raw 8-bit RGB rows here for encoding. PNG when the path
 * ends in ".png" (stb_image_write), binary PPM otherwise.
 *
 * Builtin screenshot feature: user-facing entry points are
 * lsRender::Screenshot() (per-backend) and F12 in the GLFW demo.
 */
#pragma once

#include <cstdint>

namespace lsScreenshot
{
	/**
	 * @brief Write w*h top-down RGB rows to @p path.
	 * @param path  output file; ".png" extension selects PNG, else PPM
	 * @param rgb   w*h*3 bytes, rows top-to-bottom (flip BEFORE calling
	 *              if your readback is bottom-up, e.g. glReadPixels)
	 * @return true on success
	 */
	bool saveRGB( const char* path, int w, int h, const uint8_t* rgb );
}
