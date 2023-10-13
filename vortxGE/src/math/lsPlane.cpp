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

#include "lsVector3.h"
#include "lsPlane.h"
#include "lsMath.h"


lsPlane::lsPlane()
{

}

lsPlane::lsPlane( lsVector3 &v1,  lsVector3 &v2, lsVector3 &v3)
{
	set3Points(v1,v2,v3);
}

lsPlane::~lsPlane()
{
}



void lsPlane::set3Points( lsVector3 &v1,  lsVector3 &v2,  lsVector3 &v3 )
{
	lsMath m;
	lsVector3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = aux2 * aux1;

	normal.normalize();
	point = v2;
	d = -m.Dot( normal, point );
}

void lsPlane::setNormalAndPoint( const lsVector3 &normal, const lsVector3 &point )
{
	lsMath m;

	this->normal = normal;
	this->normal.normalize();
	d = -m.Dot( normal, point );
}

void lsPlane::setCoefficients(float a, float b, float c, float d)
{
	// set the normal vector
	normal = lsVector3(a,b,c);
	float l = normal.length();
	normal.normalize();
	this->d = d/l;
}


float lsPlane::distance(const lsVector3 &p )
{
	lsMath m;
	return d + m.Dot( normal, p );
}

void lsPlane::debug()
{
    normal.debug( "Plane(" );
	cout << ") " << d << endl;

}

/*
void lsPlane::drawGuide()
{
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	// Draw line
	glBegin(GL_LINE);
	glVertex3f( point.x, point.y, point.z );
	glVertex3f( point.x + normal.x, point.y + normal.y, point.z + normal.z );
	glEnd();

}
*/
