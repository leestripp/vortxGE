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

#ifndef LSEVENT_DELNODE_H
#define LSEVENT_DELNODE_H

#include "lsEvent.h"


//! Delete Scene node
/*!
 * Event to delete a scene node, its not safe to delete a node directly at any time so add this event insted.
 *
 * */
class lsEvent_DelNode : public lsEvent
{
public:
	lsEvent_DelNode();
	virtual ~lsEvent_DelNode();

};

#endif // LSEVENT_DELNODE_H
