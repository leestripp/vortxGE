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

#ifndef LSLIGHTPTR_H_
#define LSLIGHTPTR_H_

class lsLight;



//! Light pointer
/*!
 * Holds a pointer to a light.
 * lsNodes use this.
 *
 */
class lsLightPtr
{
public:
	lsLightPtr();
	virtual ~lsLightPtr();


	lsLightPtr *getNext_lightptr() const
	{
		return next_lightptr;
	}

	void setNext_lightptr(lsLightPtr *next_lightptr)
	{
		this->next_lightptr = next_lightptr;
	}

	lsLightPtr *getPrev_lightptr() const
	{
		return prev_lightptr;
	}

	void setPrev_lightptr(lsLightPtr *prev_lightptr)
	{
		this->prev_lightptr = prev_lightptr;
	}

	lsLight *getLight() const
	{
		return light;
	}

	void setLight(lsLight *light)
	{
		this->light = light;
	}

private:
	lsLightPtr* next_lightptr;
	lsLightPtr* prev_lightptr;

	lsLight* light;

};

#endif /* LSLIGHTPTR_H_ */
