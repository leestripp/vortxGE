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

#ifndef LSANIMPROPERTY_H
#define LSANIMPROPERTY_H

#include <iostream>


const int LSANIMPROP_NONE	= 0;
const int LSANIMPROP_VECTOR2	= 1;
const int LSANIMPROP_VECTOR3	= 2;
const int LSANIMPROP_VECTOR4	= 3;
const int LSANIMPROP_FLOAT	= 4;

class lsProperty;

using namespace std;


//! Animation curve set
/*!
 * Points to a lsProperty and uses its curves to control it.
 *
 * */
class lsAnimProperty
{
public:
	lsAnimProperty( lsProperty * p );
	virtual ~lsAnimProperty();

	virtual void Update( double frame ) = 0;


	void setAnimprop_type( int val )
	{
		animprop_type = val;
	}

	int getAnimprop_type() const
	{
		return animprop_type;
	}

	void setProperty( lsProperty *val )
	{
		property = val;
	}

	lsProperty* getProperty() const
	{
		return property;
	}

	// Navi ###################################################

	void setNext_animprop( lsAnimProperty *val )
	{
		next_animprop = val;
	}

	lsAnimProperty* getNext_animprop() const
	{
		return next_animprop;
	}

	void setPrev_animprop( lsAnimProperty *val )
	{
		prev_animprop = val;
	}

	lsAnimProperty* getPrev_animprop() const
	{
		return prev_animprop;
	}


private:
	lsAnimProperty *next_animprop;
	lsAnimProperty *prev_animprop;

	lsProperty *property;
	int animprop_type;

};

#endif // LSANIMPROPERTY_H
