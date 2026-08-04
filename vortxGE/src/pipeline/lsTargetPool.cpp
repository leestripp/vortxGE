#include "lsTargetPool.h"

lsTargetPool::~lsTargetPool()
{
	for( auto& [name, e] : m_entries ) destroy( e );
}

lsFBOID lsTargetPool::acquire( const std::string& name, bool withDepth,
							   float scale )
{
	auto& e = m_entries[name];
	e.withDepth = withDepth;
	e.scale = scale;
	if( m_w && m_h && !e.realized ) realize( e );
	return e.fbo;
}

lsFBOID lsTargetPool::find( const std::string& name ) const
{
	auto it = m_entries.find( name );
	return ( it == m_entries.end() ) ? 0 : it->second.fbo;
}

lsTextureID lsTargetPool::colorTexture( const std::string& name ) const
{
	auto it = m_entries.find( name );
	return ( it == m_entries.end() ) ? 0 : it->second.color;
}

void lsTargetPool::setFramebufferSize( uint32_t w, uint32_t h )
{
	if( w == m_w && h == m_h ) return;
	m_w = w;
	m_h = h;
	for( auto& [name, e] : m_entries ) realize( e );
}

void lsTargetPool::destroy( Entry& e )
{
	if( e.fbo ) { m_render->DeleteFramebuffers( 1, &e.fbo ); e.fbo = 0; }
	if( e.color ) { m_render->DeleteTextures( 1, &e.color ); e.color = 0; }
	if( e.depthTex ) { m_render->DeleteTextures( 1, &e.depthTex ); e.depthTex = 0; }
	e.realized = false;
}

void lsTargetPool::realize( Entry& e )
{
	destroy( e );
	if( m_w == 0 || m_h == 0 ) return;

	const uint32_t w = (uint32_t)( m_w * e.scale );
	const uint32_t h = (uint32_t)( m_h * e.scale );
	if( w == 0 || h == 0 ) return;

	// colour attachment
	m_render->GenTextures( 1, &e.color );
	m_render->BindTexture( LS_TEXTURE_2D, e.color );
	m_render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR );
	m_render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
	m_render->TexImage2D( LS_TEXTURE_2D, 0, LS_RGBA, w, h, 0,
						  LS_RGBA, LS_UNSIGNED_BYTE, nullptr );

	// depth attachment
	if( e.withDepth )
	{
		m_render->GenTextures( 1, &e.depthTex );
		m_render->BindTexture( LS_TEXTURE_2D, e.depthTex );
		m_render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_NEAREST );
		m_render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_NEAREST );
		m_render->TexImage2D( LS_TEXTURE_2D, 0, LS_DEPTH_COMPONENT, w, h, 0,
							  LS_DEPTH_COMPONENT, LS_FLOAT, nullptr );
	}

	m_render->GenFramebuffers( 1, &e.fbo );
	m_render->BindFramebuffer( LS_FRAMEBUFFER, e.fbo );
	m_render->FramebufferTexture2D( LS_FRAMEBUFFER, LS_COLOR_ATTACHMENT0,
									LS_TEXTURE_2D, e.color, 0 );
	if( e.withDepth )
		m_render->FramebufferTexture2D( LS_FRAMEBUFFER, LS_DEPTH_ATTACHMENT,
										LS_TEXTURE_2D, e.depthTex, 0 );
	m_render->BindFramebuffer( LS_FRAMEBUFFER, SCREEN );

	e.realized = true;
}
