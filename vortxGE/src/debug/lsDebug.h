/**
 * @file lsDebug.h
 * @brief D-series (Plan.md): compile-time gating for built-in debug
 * and profiler tooling.
 *
 * Everything guarded by this header is COMPILED OUT in Release builds
 * (VORTX_DEBUG_TOOLS undefined): no env-var lookups, no strings, no
 * branches — zero bloat. Default is ON for Debug, OFF for Release;
 * see VORTX_DEBUG_TOOLS in vortxGE/CMakeLists.txt.
 *
 * The builtin screenshot feature (lsRender::Screenshot) is a real
 * feature and is NOT gated by this header.
 */
#pragma once

#if defined(VORTX_DEBUG_TOOLS)
	/// Execute @p x only in debug-tools builds.
#	define VORTX_DEBUG_ONLY( x ) x
#else
#	define VORTX_DEBUG_ONLY( x )
#endif
