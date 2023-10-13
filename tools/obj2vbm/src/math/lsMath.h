/*
 * Copyright (C) 2015 by Lee A. Stripp <leestripp@gmail.com>
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef LSMATH_H
#define LSMATH_H

#define lsPI 3.14159265358979323846
#define ANG2RAD 3.14159265358979323846/180.0
#define RAD2ANG 180.0/3.14159265358979323846

// Classes used
class lsVector3;

//! 3D Math functions
/*!
 * Commonly used math functions for 3D space.
 *
 * */
class lsMath
{
public:
    lsMath();
    virtual ~lsMath();

    lsVector3 Cross( const lsVector3& Vec1, const lsVector3& Vec2 );
    float Dot( const lsVector3& Vec1, const lsVector3& Vec2 );

};

#endif // LSMATH_H
