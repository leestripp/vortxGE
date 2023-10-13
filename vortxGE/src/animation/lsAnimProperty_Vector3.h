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

#ifndef LSANIMPROPERTY_VECTOR3_H
#define LSANIMPROPERTY_VECTOR3_H

#include "lsAnimProperty.h"

class lsProperty;
class lsCubicCurve;


class lsAnimProperty_Vector3 : public lsAnimProperty
{
public:
	lsAnimProperty_Vector3( lsProperty *p );
	virtual ~lsAnimProperty_Vector3();

	void Update( double frame );
	void InsertKey(double frame, const lsVector3& val);

private:
	lsCubicCurve *curve1;
	lsCubicCurve *curve2;
	lsCubicCurve *curve3;

};

#endif // LSANIMPROPERTY_VECTOR3_H
