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

#include <iostream>
#include <sstream>

#include "lsMatrix3x3.h"


lsMatrix3x3::lsMatrix3x3()
{
    // float isn't accurate so wee need this for tests.
    moe = 0.00001;

    LoadIdentity();
}

lsMatrix3x3::~lsMatrix3x3()
{
}

void lsMatrix3x3::LoadIdentity()
{
    matrix[ 0 ] = 1;
    matrix[ 1 ] = 0;
    matrix[ 2 ] = 0;

    matrix[ 3 ] = 0;
    matrix[ 4 ] = 1;
    matrix[ 5 ] = 0;

    matrix[ 6 ] = 0;
    matrix[ 7 ] = 0;
    matrix[ 8 ] = 1;
}

//! Extract a rotation matrix
/*!
 * Extract a rotation matrix from a 4x4 matrix
 * 
 * */
void lsMatrix3x3::ExtractRotationMatrix4x4( const lsMatrix4x4& mat )
{
	// X
	matrix[0] = mat.matrix[ 0];
	matrix[1] = mat.matrix[ 1];
	matrix[2] = mat.matrix[ 2];
	// Y
	matrix[3] = mat.matrix[ 4];
	matrix[4] = mat.matrix[ 5];
	matrix[5] = mat.matrix[ 6];
	// Z
	matrix[6] = mat.matrix[ 8];
	matrix[7] = mat.matrix[ 9];
    matrix[8] = mat.matrix[10];
}

/*!
 * \brief lsMatrix3x3::Transpose
 *
 * Flip rows for colums.\n
 *
 * \param mat
 */
void lsMatrix3x3::Transpose( const lsMatrix3x3 &mat )
{
    // Row 0
    matrix[0] = mat.matrix[ 0 ];
    matrix[1] = mat.matrix[ 3 ];
    matrix[2] = mat.matrix[ 6 ];
    // Row 1
    matrix[3] = mat.matrix[ 1 ];
    matrix[4] = mat.matrix[ 4 ];
    matrix[5] = mat.matrix[ 7 ];
    // Row 2
    matrix[6] = mat.matrix[ 2 ];
    matrix[7] = mat.matrix[ 5 ];
    matrix[8] = mat.matrix[ 8 ];
}

/*!
 * \brief lsMatrix3x3::Inverse
 *
 * Build the inverse matrix. If you multiply the inverse by the original matrix you get the Identity matrix.\n
 *
 * \param mat
 */
void lsMatrix3x3::Inverse( const lsMatrix3x3 &mat )
{
    // Determinant
    float Det;
    float Dp[3];
    float Dn[3];
    // Dp
    Dp[0] = mat.matrix[0] * mat.matrix[4] * mat.matrix[8];
    Dp[1] = mat.matrix[3] * mat.matrix[7] * mat.matrix[2];
    Dp[2] = mat.matrix[6] * mat.matrix[1] * mat.matrix[5];
    // Dn
    Dn[0] = mat.matrix[2] * mat.matrix[4] * mat.matrix[6];
    Dn[1] = mat.matrix[5] * mat.matrix[7] * mat.matrix[0];
    Dn[2] = mat.matrix[8] * mat.matrix[1] * mat.matrix[3];
    // Det
    Det = ( Dp[0]+Dp[1]+Dp[2] ) - ( Dn[0]+Dn[1]+Dn[2] );

    // Build Matrix of minors
    lsMatrix3x3 N;
    float mat2x2[4];
    // pos 0
    mat2x2[0] = mat.matrix[4];
    mat2x2[1] = mat.matrix[5];
    mat2x2[2] = mat.matrix[7];
    mat2x2[3] = mat.matrix[8];
    N.matrix[0] = +(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 1
    mat2x2[0] = mat.matrix[3];
    mat2x2[1] = mat.matrix[5];
    mat2x2[2] = mat.matrix[6];
    mat2x2[3] = mat.matrix[8];
    N.matrix[1] = -(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 2
    mat2x2[0] = mat.matrix[3];
    mat2x2[1] = mat.matrix[4];
    mat2x2[2] = mat.matrix[6];
    mat2x2[3] = mat.matrix[7];
    N.matrix[2] = +(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 3
    mat2x2[0] = mat.matrix[1];
    mat2x2[1] = mat.matrix[2];
    mat2x2[2] = mat.matrix[7];
    mat2x2[3] = mat.matrix[8];
    N.matrix[3] = -(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 4
    mat2x2[0] = mat.matrix[0];
    mat2x2[1] = mat.matrix[2];
    mat2x2[2] = mat.matrix[6];
    mat2x2[3] = mat.matrix[8];
    N.matrix[4] = +(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 5
    mat2x2[0] = mat.matrix[0];
    mat2x2[1] = mat.matrix[1];
    mat2x2[2] = mat.matrix[6];
    mat2x2[3] = mat.matrix[7];
    N.matrix[5] = -(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 6
    mat2x2[0] = mat.matrix[1];
    mat2x2[1] = mat.matrix[2];
    mat2x2[2] = mat.matrix[4];
    mat2x2[3] = mat.matrix[5];
    N.matrix[6] = +(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 7
    mat2x2[0] = mat.matrix[0];
    mat2x2[1] = mat.matrix[2];
    mat2x2[2] = mat.matrix[3];
    mat2x2[3] = mat.matrix[5];
    N.matrix[7] = -(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // pos 8
    mat2x2[0] = mat.matrix[0];
    mat2x2[1] = mat.matrix[1];
    mat2x2[2] = mat.matrix[3];
    mat2x2[3] = mat.matrix[4];
    N.matrix[8] = +(( mat2x2[0] * mat2x2[3] )-( mat2x2[1] * mat2x2[2] ));
    // Transpos n
    Transpose( N );
    // Devide by Det
    for( int i=0; i<9; i++ )
        matrix[i] /= Det;

    // We have our inverse
}

lsMatrix3x3 &lsMatrix3x3::operator=(const lsMatrix3x3 &m)
{
    for( int i=0; i<9; i++ )
        matrix[i] = m.matrix[i];

    return *this;
}

lsMatrix3x3 &lsMatrix3x3::operator*=(const lsMatrix3x3 &B)
{
    lsMatrix3x3 A = *this;

    for( int i = 0; i < 3; i++ )
    {
        float ai0 = A.matrix[0*3+i];
        float ai1 = A.matrix[1*3+i];
        float ai2 = A.matrix[2*3+i];

        matrix[0*3+i] = ai0 * B.matrix[ 0] + ai1 * B.matrix[ 1] + ai2 * B.matrix[ 2];
        matrix[1*3+i] = ai0 * B.matrix[ 3] + ai1 * B.matrix[ 4] + ai2 * B.matrix[ 5];
        matrix[2*3+i] = ai0 * B.matrix[ 6] + ai1 * B.matrix[ 7] + ai2 * B.matrix[ 8];
    }
    return *this;
}

lsMatrix3x3 lsMatrix3x3::operator*(const lsMatrix3x3 &B)
{
    lsMatrix3x3 ret;
    lsMatrix3x3 A = *this;

    for( int i = 0; i < 3; i++ )
    {
        float ai0 = A.matrix[0*3+i];
        float ai1 = A.matrix[1*3+i];
        float ai2 = A.matrix[2*3+i];

        ret.matrix[0*3+i] = ai0 * B.matrix[ 0] + ai1 * B.matrix[ 1] + ai2 * B.matrix[ 2];
        ret.matrix[1*3+i] = ai0 * B.matrix[ 3] + ai1 * B.matrix[ 4] + ai2 * B.matrix[ 5];
        ret.matrix[2*3+i] = ai0 * B.matrix[ 6] + ai1 * B.matrix[ 7] + ai2 * B.matrix[ 8];
    }
    return ret;
}


/*!
 * \brief lsMatrix3x3::debug
 * \param str
 */
void lsMatrix3x3::debug( const string& str )
{

    // For Thread overlap
    stringstream msg;
	
    msg << "Matrix : " << str << endl;
	
	int k;
	for( int i=0; i<3; i++ )
	{
		for( int j=0; j<9; j+=3 )
		{
			k = j+i;
			// cout << " m[" << k << "] = " << mat[k];
            msg << "[ " << matrix[k] << " ]   ";
			
		}
        msg << endl;
	}
    cout << msg.str();
}

bool lsMatrix3x3::isEqual(const lsMatrix3x3 &mat)
{
    for( int i=0; i<9; i++ )
    {
        if(( matrix[i] < (mat.matrix[i]-moe) )||( matrix[i] > (mat.matrix[i]+moe) )) return false;
    }
    return true;
}

bool lsMatrix3x3::isIdentity()
{
    // check 1's
    if(( matrix[0] < (1.0-moe) )||( matrix[0] > (1.0+moe) )) return false;
    if(( matrix[4] < (1.0-moe) )||( matrix[4] > (1.0+moe) )) return false;
    if(( matrix[8] < (1.0-moe) )||( matrix[8] > (1.0+moe))) return false;

    for( int i=0; i<9; i++ )
    {
        if((i!=0)&&(i!=4)&&(i!=8))
        {
            if(( matrix[i] < -moe )||( matrix[i] > moe )) return false;
        }
    }
    return true;
}

