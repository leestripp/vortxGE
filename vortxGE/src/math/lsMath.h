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

#ifndef LSMATH_H
#define LSMATH_H

#define lsPI 3.14159265358979323846
#define ANG2RAD 3.14159265358979323846/180.0
#define RAD2ANG 180.0/3.14159265358979323846

// array types
typedef float	Vector2f[2];
typedef float	Vector3f[3];
typedef float	Vector4f[4];
typedef float	Matrix44f[16];

// Classes used
class lsVector2;
class lsVector3;
class lsVector4;

/*!
 * \brief The lsMath class
 *
 * Commonly used math functions for 3D space.\n
 *
 */
class lsMath
{
public:
    lsMath();
    virtual ~lsMath();

    bool approxEqual( float a, float b, float epsilon );

    lsVector3 Cross( const lsVector3& Vec1, const lsVector3& Vec2 );
    float Dot( const lsVector3& Vec1, const lsVector3& Vec2 );

};

#endif // LSMATH_H
