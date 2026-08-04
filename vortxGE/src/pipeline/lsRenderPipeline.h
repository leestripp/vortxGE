/**
 * @file lsRenderPipeline.h
 * @brief Explicit, ordered render pipeline (Plan.md Part 3).
 *
 * Replaces the lsControl chain + per-control managers + lsPass linked
 * list with one flat, data-driven stage list owned by the app.
 * Everything stays above the lsRender interface — both backends work.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "lsRenderStage.h"
#include "lsTargetPool.h"

class lsRenderPipeline
{
public:
	explicit lsRenderPipeline( lsRender* render )
		: m_render( render ), m_targets( render ) {}

	lsTargetPool& targets() { return m_targets; }

	/// Add a stage; the pipeline takes ownership and returns it.
	template<typename T, typename... Args>
	T* addStage( Args&&... args )
	{
		auto s = std::make_unique<T>( std::forward<Args>( args )... );
		T* ptr = s.get();
		m_stages.push_back( std::move( s ) );
		return ptr;
	}

	bool removeStage( const std::string& name );
	lsRenderStage* findStage( const std::string& name ) const;
	size_t stageCount() const { return m_stages.size(); }

	/// Single resize owner (Plan.md Part 3 rule 5).
	void setFramebufferSize( uint32_t w, uint32_t h )
	{ m_targets.setFramebufferSize( w, h ); }

	/// Execute enabled stages in order.
	void render();

private:
	lsRender*	m_render;
	lsTargetPool m_targets;
	std::vector<std::unique_ptr<lsRenderStage>> m_stages;
};
