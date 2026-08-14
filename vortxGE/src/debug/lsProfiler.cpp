/**
 * @file lsProfiler.cpp
 * @brief D3 profiler implementation — see lsProfiler.h.
 * Empty TU in Release (whole file behind VORTX_DEBUG_TOOLS).
 */
#include "lsProfiler.h"

#if defined(VORTX_DEBUG_TOOLS)

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

lsProfiler& lsProfiler::instance()
{
	static lsProfiler s;
	return s;
}

lsProfiler::lsProfiler()
{
	const char* env = getenv( "VORTX_PROF" );
	if( ! env || ! *env ) return;
	m_active = true;
	if( strcmp( env, "1" ) != 0 )
	{
		m_csv = true;
		m_csvFile = fopen( env, "w" );
		if( m_csvFile )
			fprintf( m_csvFile, "frame,zone,cpu_ms,gpu_ms\n" );
		else
			cerr << "PROFILER: cannot open CSV '" << env << "', console mode" << endl;
	}
	cout << "PROFILER: active (" << ( m_csv ? "CSV" : "console, 120-frame summary" )
	     << ")" << endl;
}

void lsProfiler::beginCpu( const char* name )
{
	if( ! m_active ) return;
	m_open[name] = chrono::steady_clock::now();
}

void lsProfiler::endCpu( const char* name )
{
	if( ! m_active ) return;
	auto it = m_open.find( name );
	if( it == m_open.end() ) return;
	const double ms = chrono::duration<double, milli>(
		chrono::steady_clock::now() - it->second ).count();
	m_open.erase( it );
	Zone& z = m_zones[name];
	if( z.n == 0 && z.frameMs == 0.0 ) m_order.push_back( name );
	z.frameMs += ms;
}

void lsProfiler::recordGpu( const std::string& name, double ms )
{
	if( ! m_active ) return;
	Zone& z = m_zones[name];
	if( z.n == 0 && z.frameMs == 0.0 && ! z.hasGpu ) m_order.push_back( name );
	z.gpuFrameMs = ms;
	z.hasGpu = true;
}

void lsProfiler::frameTick()
{
	if( ! m_active ) return;
	m_frame++;

	if( m_csv && m_csvFile )
		for( const auto& name : m_order )
		{
			const Zone& z = m_zones[name];
			fprintf( m_csvFile, "%llu,%s,%.4f,%.4f\n",
				(unsigned long long)m_frame, name.c_str(),
				z.frameMs, z.hasGpu ? z.gpuFrameMs : 0.0 );
		}

	for( const auto& name : m_order )
	{
		Zone& z = m_zones[name];
		z.cpuSum += z.frameMs; if( z.frameMs > z.cpuMax ) z.cpuMax = z.frameMs;
		z.n++;
		if( z.hasGpu )
		{
			z.gpuSum += z.gpuFrameMs;
			if( z.gpuFrameMs > z.gpuMax ) z.gpuMax = z.gpuFrameMs;
			z.gpuN++;
		}
		z.frameMs = 0.0; z.gpuFrameMs = 0.0; z.hasGpu = false;
	}

	if( ! m_csv && m_frame % 120 == 0 ) report();
	if( m_frame % 120 == 0 ) // reset the rolling window after a report
		for( auto& [n, z] : m_zones )
		{
			z.cpuSum = z.cpuMax = 0; z.n = 0;
			z.gpuSum = z.gpuMax = 0; z.gpuN = 0;
		}
}

void lsProfiler::report()
{
	cout << "PROFILER (120 frames):" << endl;
	cout << "  zone                       CPU avg     max  |  GPU avg     max  (ms)" << endl;
	for( const auto& name : m_order )
	{
		const Zone& z = m_zones[name];
		if( z.n == 0 ) continue;
		char line[160];
		if( z.gpuN )
			snprintf( line, sizeof line, "  %-24s %8.3f %8.3f  | %8.3f %8.3f",
				name.c_str(), z.cpuSum / z.n, z.cpuMax,
				z.gpuSum / z.gpuN, z.gpuMax );
		else
			snprintf( line, sizeof line, "  %-24s %8.3f %8.3f  |       -       -",
				name.c_str(), z.cpuSum / z.n, z.cpuMax );
		cout << line << endl;
	}
}

#endif // VORTX_DEBUG_TOOLS
