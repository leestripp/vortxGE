// lsStreamingManager — node-driven asset residency (Plan.md Phase 7 M3)
// See lsStreamingManager.h for the design contract.

#include "lsStreamingManager.h"

#include "../scene/lsNode.h"
#include "lsAsset.h"

#include <algorithm>

void lsStreamingManager::registerNode( lsNode* node, lsAsset* asset )
{
	if (!node || !asset) return;
	// Ignore duplicate registrations of the same pair.
	for (const auto& e : m_entries)
		if (e.node == node && e.asset == asset) return;

	Entry e;
	e.node = node;
	e.asset = asset;
	e.resident = (asset->getData() != nullptr);
	e.baselineUsers = asset->getUsers();
	m_entries.push_back( e );
}

size_t lsStreamingManager::residentCount() const
{
	size_t n = 0;
	for (const auto& e : m_entries)
		if (e.resident) ++n;
	return n;
}

// World-space node position: translation column of the model matrix
// (same pattern as lsLightManager::setActiveLights).
static lsVector3 nodeWorldPos( const lsNode* node )
{
	return lsVector3( node->ModelMatrix.matrix[12],
					  node->ModelMatrix.matrix[13],
					  node->ModelMatrix.matrix[14] );
}

void lsStreamingManager::update( const lsVector3& camPos )
{
	// 1. Distance refresh + hysteresis decisions.
	for (auto& e : m_entries)
	{
		e.lastDistance = (nodeWorldPos( e.node ) - camPos).length();

		if (e.resident && e.lastDistance > m_config.unloadRadius)
		{
			// Ref-aware: only evict while no extra users appeared.
			if (e.asset->getUsers() <= e.baselineUsers)
			{
				e.asset->FreeAsset();
				e.resident = false;
			}
		}
		else if (!e.resident && e.lastDistance < m_config.loadRadius)
		{
			e.asset->AllocAsset();
			e.resident = true;
		}
	}

	// 2. Budget enforcement — farthest-first eviction until under budget.
	if (residentCount() > m_config.maxResident)
	{
		// Eviction candidates sorted farthest-first.
		std::vector<Entry*> candidates;
		for (auto& e : m_entries)
			if (e.resident && e.asset->getUsers() <= e.baselineUsers)
				candidates.push_back( &e );
		std::sort( candidates.begin(), candidates.end(),
				   []( const Entry* a, const Entry* b ) {
					   return a->lastDistance > b->lastDistance;
				   } );

		for (Entry* e : candidates)
		{
			if (residentCount() <= m_config.maxResident) break;
			e->asset->FreeAsset();
			e->resident = false;
		}
	}
}
