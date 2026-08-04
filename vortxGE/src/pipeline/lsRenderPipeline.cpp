#include "lsRenderPipeline.h"

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
	for( auto& s : m_stages )
		if( s->isEnabled() )
			s->execute( m_render, m_targets );
}
