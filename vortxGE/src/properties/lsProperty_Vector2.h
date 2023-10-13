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

#ifndef LSPROPERTY_VECTOR2_H
#define LSPROPERTY_VECTOR2_H

#include "lsProperty.h"


class lsVector2;


class lsProperty_Vector2 : public lsProperty
{
public:
	lsProperty_Vector2( lsVector2 *val );
	virtual ~lsProperty_Vector2();

	void Set( float x, float y );

    const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	float Get_Y();

	void Set_X(float val);
	void Set_Y(float val);


private:
	lsVector2 *vec2;

	// toString
	string output;

};

#endif // LSPROPERTY_VECTOR2_H
