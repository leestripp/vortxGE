// Tests for vortxGE/src/assets/lsStreamingManager.{h,cpp} (Plan.md Phase 7 M3)
// Pure CPU — mocked assets and minimal concrete nodes, no GL context.
//
// Covers: registration (baseline users, dedup, null-guard), distance-driven
// load/evict, hysteresis band stability, ref-aware eviction, and budget
// enforcement (farthest-first, ref-locked skipped).

#include "doctest/doctest.h"

#include "assets/lsStreamingManager.h"
#include "assets/lsAsset.h"
#include "scene/lsNode.h"

using doctest::Approx;

namespace {

// lsAsset ctor takes lsRender* (nullptr here); only Alloc/Free are pure.
class MockAsset : public lsAsset
{
public:
	MockAsset() : lsAsset( nullptr ) {}
	~MockAsset() override { setData( nullptr ); }	// stop ~lsAsset deleting the fake pointer
	void AllocAsset() override { allocCount++; setData( fake ); }
	void FreeAsset() override { freeCount++; setData( nullptr ); }
	int allocCount = 0;
	int freeCount = 0;
private:
	lsData* fake = reinterpret_cast<lsData*>( (uintptr_t)0x1 );	// never dereferenced
};

// lsNode is abstract (pure virtual toString()) — minimal concrete node.
class StreamTestNode : public lsNode
{
public:
	StreamTestNode() : lsNode( nullptr ) {}
	const std::string& toString() override { static const std::string s = "StreamTestNode"; return s; }
	// Place the node in world space via the model-matrix translation column
	// (same layout the manager reads).
	void setWorldPos( float x, float y, float z )
	{
		ModelMatrix.matrix[12] = x;
		ModelMatrix.matrix[13] = y;
		ModelMatrix.matrix[14] = z;
	}
};

} // namespace

TEST_CASE( "lsStreamingManager: registration" )
{
	lsStreamingManager sm;
	StreamTestNode node;
	MockAsset asset;

	SUBCASE( "records baseline users and initial residency from getData" )
	{
		asset.AddUser();				// as Set_Asset() would
		asset.AllocAsset();				// starts resident
		sm.registerNode( &node, &asset );
		REQUIRE( sm.entries().size() == 1 );
		CHECK( sm.entries()[0].baselineUsers == 1u );
		CHECK( sm.entries()[0].resident );
	}
	SUBCASE( "not-allocated asset registers as non-resident" )
	{
		sm.registerNode( &node, &asset );
		CHECK_FALSE( sm.entries()[0].resident );
	}
	SUBCASE( "duplicate pair ignored" )
	{
		sm.registerNode( &node, &asset );
		sm.registerNode( &node, &asset );
		CHECK( sm.entries().size() == 1 );
	}
	SUBCASE( "null arguments ignored" )
	{
		sm.registerNode( nullptr, &asset );
		sm.registerNode( &node, nullptr );
		CHECK( sm.entries().empty() );
	}
}

TEST_CASE( "lsStreamingManager: distance-driven load and evict" )
{
	lsResidencyConfig cfg;
	cfg.loadRadius = 10.0f;
	cfg.unloadRadius = 20.0f;
	lsStreamingManager sm( cfg );

	StreamTestNode node;
	MockAsset asset;
	sm.registerNode( &node, &asset );	// starts non-resident (no data)
	const lsVector3 camPos( 0.0f, 0.0f, 0.0f );

	SUBCASE( "node within loadRadius → AllocAsset" )
	{
		node.setWorldPos( 5.0f, 0.0f, 0.0f );
		sm.update( camPos );
		CHECK( asset.allocCount == 1 );
		CHECK( sm.entries()[0].resident );
		CHECK( sm.entries()[0].lastDistance == Approx( 5.0f ) );
	}
	SUBCASE( "node beyond unloadRadius → nothing loads" )
	{
		node.setWorldPos( 100.0f, 0.0f, 0.0f );
		sm.update( camPos );
		CHECK( asset.allocCount == 0 );
		CHECK_FALSE( sm.entries()[0].resident );
	}
	SUBCASE( "resident asset evicted when node leaves unloadRadius" )
	{
		node.setWorldPos( 5.0f, 0.0f, 0.0f );
		sm.update( camPos );
		REQUIRE( sm.entries()[0].resident );
		node.setWorldPos( 50.0f, 0.0f, 0.0f );
		sm.update( camPos );
		CHECK( asset.freeCount == 1 );
		CHECK_FALSE( sm.entries()[0].resident );
	}
	SUBCASE( "hysteresis band: no flapping between the radii" )
	{
		node.setWorldPos( 5.0f, 0.0f, 0.0f );	// load
		sm.update( camPos );
		REQUIRE( sm.entries()[0].resident );
		node.setWorldPos( 15.0f, 0.0f, 0.0f );	// in band — stays resident
		sm.update( camPos );
		CHECK( asset.freeCount == 0 );
		CHECK( sm.entries()[0].resident );

		node.setWorldPos( 50.0f, 0.0f, 0.0f );	// evict
		sm.update( camPos );
		REQUIRE_FALSE( sm.entries()[0].resident );
		node.setWorldPos( 15.0f, 0.0f, 0.0f );	// in band — stays evicted
		sm.update( camPos );
		CHECK( asset.allocCount == 1 );
		CHECK_FALSE( sm.entries()[0].resident );
	}
}

TEST_CASE( "lsStreamingManager: ref-aware eviction" )
{
	lsResidencyConfig cfg;
	cfg.loadRadius = 10.0f;
	cfg.unloadRadius = 20.0f;
	lsStreamingManager sm( cfg );

	StreamTestNode node;
	MockAsset asset;
	asset.AddUser();						// node's own reference (baseline 1)
	sm.registerNode( &node, &asset );
	const lsVector3 camPos( 0.0f, 0.0f, 0.0f );

	node.setWorldPos( 5.0f, 0.0f, 0.0f );
	sm.update( camPos );
	REQUIRE( sm.entries()[0].resident );

	SUBCASE( "extra user blocks eviction beyond unloadRadius" )
	{
		asset.AddUser();					// someone else shares the asset
		node.setWorldPos( 100.0f, 0.0f, 0.0f );
		sm.update( camPos );
		CHECK( asset.freeCount == 0 );
		CHECK( sm.entries()[0].resident );	// still resident, protected
	}
	SUBCASE( "eviction allowed again once the extra user leaves" )
	{
		asset.AddUser();
		node.setWorldPos( 100.0f, 0.0f, 0.0f );
		sm.update( camPos );
		REQUIRE( asset.freeCount == 0 );
		asset.RemoveUser();					// back to baseline
		sm.update( camPos );
		CHECK( asset.freeCount == 1 );
		CHECK_FALSE( sm.entries()[0].resident );
	}
}

TEST_CASE( "lsStreamingManager: budget enforcement" )
{
	lsResidencyConfig cfg;
	cfg.loadRadius = 100.0f;				// everything loads
	cfg.unloadRadius = 1000.0f;				// nothing evicts by distance
	cfg.maxResident = 2;
	lsStreamingManager sm( cfg );
	const lsVector3 camPos( 0.0f, 0.0f, 0.0f );

	StreamTestNode nearN, midN, farN;
	MockAsset nearA, midA, farA;
	nearN.setWorldPos( 10.0f, 0.0f, 0.0f );
	midN.setWorldPos( 20.0f, 0.0f, 0.0f );
	farN.setWorldPos( 30.0f, 0.0f, 0.0f );
	sm.registerNode( &nearN, &nearA );
	sm.registerNode( &midN, &midA );
	sm.registerNode( &farN, &farA );

	SUBCASE( "over budget → farthest evicted first" )
	{
		sm.update( camPos );
		CHECK( sm.residentCount() == 2 );
		CHECK( nearA.freeCount == 0 );		// closest survives
		CHECK( midA.freeCount == 0 );
		CHECK( farA.freeCount == 1 );		// farthest evicted
	}
	SUBCASE( "ref-locked farthest is skipped, next farthest evicted" )
	{
		farA.AddUser();						// lock the farthest
		farA.AddUser();						// baseline 0 → any users block
		sm.update( camPos );
		CHECK( sm.residentCount() == 2 );
		CHECK( farA.freeCount == 0 );		// protected
		CHECK( midA.freeCount == 1 );		// next farthest takes the hit
		CHECK( nearA.freeCount == 0 );
	}
	SUBCASE( "new close node pushes out the now-farthest resident" )
	{
		sm.update( camPos );				// far evicted, near+mid resident
		REQUIRE( sm.residentCount() == 2 );
		StreamTestNode newN;
		MockAsset newA;
		newN.setWorldPos( 1.0f, 0.0f, 0.0f );
		sm.registerNode( &newN, &newA );
		sm.update( camPos );				// new loads → 3 → evict farthest (mid)
		CHECK( newA.allocCount == 1 );
		CHECK( midA.freeCount == 1 );
		CHECK( nearA.freeCount == 0 );
		CHECK( sm.residentCount() == 2 );
	}
}

TEST_CASE( "lsStreamingManager: empty update is a no-op" )
{
	lsStreamingManager sm;
	sm.update( lsVector3( 0.0f, 0.0f, 0.0f ) );
	CHECK( sm.entries().empty() );
	CHECK( sm.residentCount() == 0 );
}
