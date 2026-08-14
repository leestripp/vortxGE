/**
 * @file lsProfiler.h
 * @brief D3 (Plan.md): built-in CPU/GPU profiler.
 *
 * Scoped CPU zones (lsProfileZone RAII) + per-stage GPU timings fed by
 * the render backends (Vulkan timestamp queries / GL timer queries,
 * 1-frame readback latency).
 *
 * Output: rolling 120-frame console summary (VORTX_PROF=1) or CSV
 * (VORTX_PROF=path.csv). Everything compiles to NOTHING in Release
 * (VORTX_DEBUG_TOOLS off): no env lookup, no strings, no branches.
 */
#pragma once

#include "lsDebug.h"

#if defined(VORTX_DEBUG_TOOLS)

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

class lsProfiler
{
public:
	static lsProfiler& instance();

	bool active() const { return m_active; }

	/// Scoped CPU zone (non-reentrant per name — zones are flat).
	void beginCpu( const char* name );
	void endCpu( const char* name );

	/// Backends feed measured GPU time for a named stage (ms).
	void recordGpu( const std::string& name, double ms );

	/// End of frame: roll stats, write CSV rows, report every 120.
	void frameTick();

private:
	lsProfiler(); // reads VORTX_PROF

	struct Zone {
		double frameMs = 0.0;              // this frame (CPU)
		double gpuFrameMs = 0.0;           // this frame (GPU)
		bool   hasGpu = false;
		// rolling window
		double cpuSum = 0, cpuMax = 0;
		double gpuSum = 0, gpuMax = 0;
		uint32_t n = 0, gpuN = 0;
	};

	bool m_active = false;
	bool m_csv = false;
	FILE* m_csvFile = nullptr;
	uint64_t m_frame = 0;
	std::unordered_map<std::string, Zone> m_zones;
	std::unordered_map<std::string,
		std::chrono::steady_clock::time_point> m_open;
	std::vector<std::string> m_order; // first-seen order for stable reports

	void report();
};

/// RAII scoped zone. Use via VORTX_PROFILE_ZONE("name").
class lsProfileZone
{
public:
	explicit lsProfileZone( const char* name ) : m_name( name )
	{ lsProfiler::instance().beginCpu( m_name ); }
	~lsProfileZone()
	{ lsProfiler::instance().endCpu( m_name ); }
private:
	const char* m_name;
};

#endif // VORTX_DEBUG_TOOLS

/// Scoped CPU zone — vanishes entirely in Release.
#if defined(VORTX_DEBUG_TOOLS)
#	define VORTX_PROFILE_ZONE( name ) lsProfileZone _vortx_prof_zone( name )
#else
#	define VORTX_PROFILE_ZONE( name )
#endif
