// Tests for vortxGE/src/events/lsEventManager.{h,cpp} (Plan.md Phase 5
// follow-up — event queue/dispatch). Pure CPU — no GL context needed.

#include "doctest/doctest.h"

#include "events/lsEventManager.h"
#include "events/lsEvent.h"

namespace {

class CountingEvent : public lsEvent
{
public:
	void ProcessEvent() override { processCount++; }
	int processCount = 0;
};

} // namespace

TEST_CASE( "lsEventManager: queue behaviour" )
{
	lsEventManager mgr;

	SUBCASE( "empty queue polls nullptr" )
	{
		CHECK( mgr.Poll_Event() == nullptr );
	}
	SUBCASE( "FIFO order — oldest event comes out first" )
	{
		lsEvent* a = new lsEvent();
		lsEvent* b = new lsEvent();
		lsEvent* c = new lsEvent();
		mgr.Add_Event( a );
		mgr.Add_Event( b );
		mgr.Add_Event( c );

		CHECK( mgr.Poll_Event() == a );
		CHECK( mgr.Poll_Event() == b );
		CHECK( mgr.Poll_Event() == c );
		CHECK( mgr.Poll_Event() == nullptr );	// drained

		// No deletes here: lsEventManager owns queued events. Each
		// Poll_Event() deletes the previously returned event, and the
		// manager destructor deletes the final one.
	}
	SUBCASE( "dispatch: polled events process exactly once" )
	{
		CountingEvent* e1 = new CountingEvent();
		CountingEvent* e2 = new CountingEvent();
		mgr.Add_Event( e1 );
		mgr.Add_Event( e2 );

		// Usage pattern from the header docs: poll until empty, process each
		int processed = 0;
		lsEvent* e;
		while (( e = mgr.Poll_Event() ) != nullptr)
		{
			e->ProcessEvent();
			processed++;
			// No delete: manager auto-deletes on next poll / destruction.
		}
		CHECK( processed == 2 );
	}
	SUBCASE( "event type metadata round-trips" )
	{
		lsEvent e;
		e.setEvent_type( 99 );
		CHECK( e.getEvent_type() == 99 );
	}
}
