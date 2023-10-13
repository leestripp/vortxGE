/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

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
