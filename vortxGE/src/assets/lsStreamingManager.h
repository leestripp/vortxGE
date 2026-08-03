#ifndef LSSTREAMINGMANAGER_H
#define LSSTREAMINGMANAGER_H

// lsStreamingManager — node-driven asset residency (Plan.md Phase 7 M3).
//
// Registers (node, asset) pairs and, each frame, decides which assets stay
// resident based on node distance to the camera:
//   - LOAD   when the node comes within loadRadius
//   - EVICT  when it moves beyond unloadRadius (hysteresis band between the
//            two radii prevents thrash at the boundary)
//   - budget: at most maxResident assets resident; farthest-first eviction
//   - ref-aware: an asset is only evicted while its user count is at or
//     below the count recorded at registration (assets shared with other
//     systems — extra AddUser() calls — are never evicted)
//
// Eviction is FreeAsset() in place: the node keeps its asset pointer and
// user reference; lsObject::Draw already skips assets whose getData() is
// null, so evicted objects simply don't render until re-loaded.
//
// The policy is pure logic and unit-tested with mocked nodes/assets
// (tests/src/assets/lsStreamingManager_tests.cpp).

#include <cstddef>
#include <vector>

#include "../math/lsVector3.h"

class lsNode;
class lsAsset;
class lsAsyncFileLoader;

/** Residency tuning. unloadRadius must be >= loadRadius (hysteresis). */
struct lsResidencyConfig
{
	float loadRadius = 30.0f;
	float unloadRadius = 45.0f;
	size_t maxResident = 8;
};

class lsStreamingManager
{
public:
	lsStreamingManager() = default;
	explicit lsStreamingManager( const lsResidencyConfig& config ) : m_config( config ) {}

	/** Register a (node, asset) pair for distance-driven residency.
	 *  Call AFTER any Set_Asset()/AddUser() so the recorded baseline user
	 *  count includes the node's own reference. */
	void registerNode( lsNode* node, lsAsset* asset );

	/** Per-frame update — call from the scene pass with the camera position.
	 *  Applies LOAD/EVICT decisions synchronously (GL-thread safe point). */
	void update( const lsVector3& camPos );

	/** Optional async loader (M5). When set, LOAD actions queue
	 *  AllocAsset_Async and residency flips on AsyncPoll; when null,
	 *  loads are synchronous (previous behaviour). Not owned. */
	void setLoader( lsAsyncFileLoader* loader ) { m_loader = loader; }

	lsResidencyConfig& config() { return m_config; }
	const lsResidencyConfig& config() const { return m_config; }

	//---- Test/introspection interface -------------------------------------

	struct Entry
	{
		lsNode* node = nullptr;
		lsAsset* asset = nullptr;
		bool resident = false;
		bool loading = false;			///< async load in flight (M5)
		unsigned int baselineUsers = 0;
		float lastDistance = 0.0f;
	};
	const std::vector<Entry>& entries() const { return m_entries; }
	size_t residentCount() const;

private:
	std::vector<Entry> m_entries;
	lsResidencyConfig m_config;
	lsAsyncFileLoader* m_loader = nullptr;
};

#endif // LSSTREAMINGMANAGER_H
