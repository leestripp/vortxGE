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

// math
#include "../math/lsVector3.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector3.h"

#include "lsAnimProperty_Vector3.h"
#include "lsCubicCurve.h"


lsAnimProperty_Vector3::lsAnimProperty_Vector3( lsProperty * p ) : lsAnimProperty(p)
{
	setAnimprop_type( LSANIMPROP_VECTOR3 );

	// Add 3 curves
	curve1 = new lsCubicCurve();
	curve2 = new lsCubicCurve();
	curve3 = new lsCubicCurve();

}

lsAnimProperty_Vector3::~lsAnimProperty_Vector3()
{
	if( curve1 ) delete curve1;
	if( curve2 ) delete curve2;
	if( curve3 ) delete curve3;
}

void lsAnimProperty_Vector3::Update( double frame )
{
    // No animation
    if( curve1->Empty() && curve2->Empty() && curve3->Empty() ) return;

	lsProperty_Vector3 *pv3;

	pv3 = (lsProperty_Vector3*)getProperty();
	if( pv3 )
    {
		pv3->Set( curve1->Get_Value( frame ), curve2->Get_Value( frame ), curve3->Get_Value( frame ) );
	}
}

void lsAnimProperty_Vector3::InsertKey(double frame, const lsVector3& val)
{
	curve1->Add_Key( frame, val.x );
	curve2->Add_Key( frame, val.y );
	curve3->Add_Key( frame, val.z );

}



