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

#ifndef LSPROPERTY_FLOAT_H
#define LSPROPERTY_FLOAT_H

#include <sstream>
#include <string>

#include "lsProperty.h"

using namespace std;

//! Float values
/*!
 * Used by the animation system to modigy float values.
 *
 * */
class lsProperty_Float : public lsProperty
{
public:
	lsProperty_Float( float *val );
	virtual ~lsProperty_Float();

	void Set( float x );

    // lsProperty
	const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	void Set_X(float val);

private:
	float *vec1;

	// toString
	string output;

};

#endif // LSPROPERTY_FLOAT_H
