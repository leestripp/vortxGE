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

#ifndef LSMATRIX3X3_H
#define LSMATRIX3X3_H

#include <iostream>
#include <string>

#include "lsMatrix4x4.h"

// array types
typedef float	Matrix33f[9];

using namespace std;


//! Matrix
/*!
 * Holds a 3x3 Matrix in the OpenGL format
 *
 */
class lsMatrix3x3
{
public:
    lsMatrix3x3();
    virtual ~lsMatrix3x3();

    void LoadIdentity();
    void Transpose( const lsMatrix3x3& mat );
    void Inverse( const lsMatrix3x3& mat );
    // checks
    bool isEqual( const lsMatrix3x3& mat );
    bool isIdentity();

    lsMatrix3x3& operator=(const lsMatrix3x3& m);
    lsMatrix3x3& operator*=( const lsMatrix3x3& B );
    lsMatrix3x3& operator*( const lsMatrix3x3& B );

    // convert 4x4
    void ExtractRotationMatrix4x4( const lsMatrix4x4& mat );

    // debug
    void debug( const string& str );

    // data
    float matrix[9];
	
private:
    float moe; // margin of error
};

#endif // LSMATRIX3X3_H
