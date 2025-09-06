#ifndef LSEVENTMANAGER_H_
#define LSEVENTMANAGER_H_

class lsScene;
class lsEvent;


//! Manage events
/*!
 * Manage core game engine events
 *
 * */
class lsEventManager
{
public:
	lsEventManager();
	virtual ~lsEventManager();

	void Add_Event( lsEvent* e );
    lsEvent* Poll_Event();

private:
	lsEvent* event_list;
	lsEvent* last_event;
	lsEvent* current_event;

};

#endif /* LSEVENTMANAGER_H_ */
