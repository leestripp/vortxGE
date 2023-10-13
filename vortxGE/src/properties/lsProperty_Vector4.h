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

#ifndef LSPROPERTY_VECTOR4_H
#define LSPROPERTY_VECTOR4_H

#include "lsProperty.h"

class lsVector4;

//! lsVector4 Property
/*!
 * Modify the content of a lsvector4 variable.
 *
 * */
class lsProperty_Vector4 : public lsProperty
{
public:
	lsProperty_Vector4( lsVector4 *val );
	virtual ~lsProperty_Vector4();

	void Set( float x, float y, float z, float w );

    const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	float Get_Y();
	float Get_Z();
	float Get_W();

	void Set_X(float val);
	void Set_Y(float val);
	void Set_Z(float val);
	void Set_W(float val);


private:
	lsVector4 *vec4;

	// toString
	string output;
};

#endif // LSPROPERTY_VECTOR4_H
