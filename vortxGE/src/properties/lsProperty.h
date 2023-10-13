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

#ifndef LSPROPERTY_H
#define LSPROPERTY_H

#include <iostream>
#include <string>

// Property types
const int LSPROP_BASE		= 0;
const int LSPROP_FLOAT		= 1;
const int LSPROP_INT		= 2;
const int LSPROP_VECTOR2	= 3;
const int LSPROP_VECTOR3	= 4;
const int LSPROP_VECTOR4	= 5;

// Property IDs
const int LSPROPID_NONE         = 0;
const int LSPROPID_POSITION     = 1;
const int LSPROPID_ROTATION     = 2;
const int LSPROPID_SCALE        = 3;
const int LSPROPID_DIFFUSE      = 4;
const int LSPROPID_SPECULAR     = 5;
const int LSPROPID_AMBIENT      = 6;
const int LSPROPID_SHININESS	= 7;
const int LSPROPID_GLOW         = 8;
const int LSPROPID_CAMERA_FOV   = 9;
const int LSPROPID_CAMERA_RATIO = 10;
const int LSPROPID_CAMERA_NEAR  = 11;
const int LSPROPID_CAMERA_FAR   = 12;
const int LSPROPID_CAMERA_ORTHOCUBE   = 13;

using namespace std;

//! Base Property
/*!
 * Subclass this to make new properties
 *
 * */
class lsProperty
{
public:
	lsProperty();
	virtual ~lsProperty();

	virtual const string& toString() = 0;
	virtual const string& fromString( const string& str ) = 0;

	// inline
	void setProp_type( int val )
	{
		prop_type = val;
	}

	int getProp_type() const
	{
		return prop_type;
	}

	void setProp_id( int val )
	{
		prop_id = val;
	}

	int getProp_id() const
	{
		return prop_id;
	}

	void setProp_name( const string& val )
	{
		name = val;
	}

	string& getProp_name()
	{
		return name;
	}

	// Navi
	void setNext_prop( lsProperty *val )
	{
		next_prop = val;
	}

	lsProperty* getNext_prop() const
	{
		return next_prop;
	}

	void setPrev_prop( lsProperty *val )
	{
		prev_prop = val;
	}

	lsProperty* getPrev_prop() const
	{
		return prev_prop;
	}


private:
	lsProperty *next_prop;
	lsProperty *prev_prop;

	int prop_type;
	int prop_id;
	string name;

};

#endif // LSPROPERTY_H
