#include "lsRenderPipeline.h"

#include "../debug/lsProfiler.h"

bool lsRenderPipeline::removeStage( const std::string& name )
{
	for( auto it = m_stages.begin(); it != m_stages.end(); ++it )
		if( ( *it )->name() == name ) { m_stages.erase( it ); return true; }
	return false;
}

lsRenderStage* lsRenderPipeline::findStage( const std::string& name ) const
{
	for( const auto& s : m_stages )
		if( s->name() == name ) return s.get();
	return nullptr;
}

void lsRenderPipeline::render()
{
#if defined(VORTX_DEBUG_TOOLS)
	const bool prof = lsProfiler::instance().active();
	if( prof ) m_render->profilerFrameBegin();
#endif
	for( auto& s : m_stages )
		if( s->isEnabled() )
		{
#if defined(VORTX_DEBUG_TOOLS)
			if( prof )
			{
				const std::string zn = "stage:" + s->name();
				m_render->profilerGpuBegin( zn );
				lsProfiler::instance().beginCpu( zn.c_str() );
				s->execute( m_render, m_targets );
				lsProfiler::instance().endCpu( zn.c_str() );
				m_render->profilerGpuEnd( zn );
				continue;
			}
#endif
			s->execute( m_render, m_targets );
		}
#if defined(VORTX_DEBUG_TOOLS)
	if( prof ) lsProfiler::instance().frameTick();
#endif
}
