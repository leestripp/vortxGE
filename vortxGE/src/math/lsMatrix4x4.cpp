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
#include <string.h>

#include "lsMatrix4x4.h"
#include "lsVector3.h"
#include "lsQuaternion.h"
#include "lsMath.h"


lsMatrix4x4::lsMatrix4x4()
{
    // margin of error
    moe = 0.00001;

    LoadIdentity();
}

lsMatrix4x4::lsMatrix4x4(const float mat4[16])
{
    memcpy( matrix, mat4, sizeof(float)*16 );
}

lsMatrix4x4::lsMatrix4x4(const lsQuaternion& q, float x, float y, float z)
{
    Set(q,x,y,z);
}

lsMatrix4x4::~lsMatrix4x4()
{
}

/*!
 * \brief lsMatrix4x4::LoadIdentity
 *
 * Load the Identity Matrix.\n
 *
 */
void lsMatrix4x4::LoadIdentity()
{
    // identity
    matrix[ 0]=1;
    matrix[ 1]=0;
    matrix[ 2]=0;
    matrix[ 3]=0;

    matrix[ 4]=0;
    matrix[ 5]=1;
    matrix[ 6]=0;
    matrix[ 7]=0;

    matrix[ 8]=0;
    matrix[ 9]=0;
    matrix[10]=1;
    matrix[11]=0;

    matrix[12]=0;
    matrix[13]=0;
    matrix[14]=0;
    matrix[15]=1;
}

/*!
 * \brief lsMatrix4x4::Rotate
 *
 * Rotate a Vector by this Matrix.\n
 *
 * \param v
 * \return
 */
lsVector3 lsMatrix4x4::Rotate(const lsVector3& v)
{
    lsVector3 ret;

    ret.x = matrix[ 0]*v.x + matrix[ 1]*v.y + matrix[ 2]*v.z;
    ret.y = matrix[ 4]*v.x + matrix[ 5]*v.y + matrix[ 6]*v.z;
    ret.z = matrix[ 8]*v.x + matrix[ 9]*v.y + matrix[10]*v.z;

    return ret;
}

void lsMatrix4x4::Set( const lsQuaternion& q, float x, float y, float z)
{
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;

    float wx = q.w*x2;
    float wy = q.w*y2;
    float wz = q.w*z2;

    float xx = q.x*x2;
    float xy = q.x*y2;
    float xz = q.x*z2;

    float yy = q.y*y2;
    float yz = q.y*z2;

    float zz = q.z*z2;

    matrix[ 0] = 1 - (yy + zz);
    matrix[ 1] = xy - wz;
    matrix[ 2] = xz + wy;
    matrix[ 3] = x;

    matrix[ 4] = xy + wz;
    matrix[ 5] = 1 - (xx + zz);
    matrix[ 6] = yz - wx;
    matrix[ 7] = y;

    matrix[ 8] = xz - wy;
    matrix[ 9] = yz + wx;
    matrix[10]= 1 - (xx + yy);
    matrix[11]= z;

    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}


lsMatrix4x4& lsMatrix4x4::operator=( const lsMatrix4x4& m )
{
    for( int i=0; i<16; i++ )
        matrix[i] = m.matrix[i];

    return *this;
}

lsMatrix4x4& lsMatrix4x4::operator*=( const lsMatrix4x4& B )
{
    lsMatrix4x4 A = *this;

    for( int i = 0; i < 4; i++ )
    {
        float ai0 = A.matrix[0*4+i],  ai1 = A.matrix[1*4+i],  ai2 = A.matrix[2*4+i],  ai3 = A.matrix[3*4+i];

        matrix[0*4+i] = ai0 * B.matrix[ 0] + ai1 * B.matrix[ 1] + ai2 * B.matrix[ 2] + ai3 * B.matrix[ 3];
        matrix[1*4+i] = ai0 * B.matrix[ 4] + ai1 * B.matrix[ 5] + ai2 * B.matrix[ 6] + ai3 * B.matrix[ 7];
        matrix[2*4+i] = ai0 * B.matrix[ 8] + ai1 * B.matrix[ 9] + ai2 * B.matrix[10] + ai3 * B.matrix[11];
        matrix[3*4+i] = ai0 * B.matrix[12] + ai1 * B.matrix[13] + ai2 * B.matrix[14] + ai3 * B.matrix[15];
    }
    return *this;
}


/*!
 * \brief lsMatrix4x4::operator *
 *
 *
 * \param B
 * \return
 */
lsMatrix4x4 lsMatrix4x4::operator*( const lsMatrix4x4& B )
{
    lsMatrix4x4 ret;
    lsMatrix4x4 A = *this;

    for( int i = 0; i < 4; i++ )
    {
        float ai0 = A.matrix[0*4+i],  ai1 = A.matrix[1*4+i],  ai2 = A.matrix[2*4+i],  ai3 = A.matrix[3*4+i];

        ret.matrix[0*4+i] = ai0 * B.matrix[ 0] + ai1 * B.matrix[ 1] + ai2 * B.matrix[ 2] + ai3 * B.matrix[ 3];
        ret.matrix[1*4+i] = ai0 * B.matrix[ 4] + ai1 * B.matrix[ 5] + ai2 * B.matrix[ 6] + ai3 * B.matrix[ 7];
        ret.matrix[2*4+i] = ai0 * B.matrix[ 8] + ai1 * B.matrix[ 9] + ai2 * B.matrix[10] + ai3 * B.matrix[11];
        ret.matrix[3*4+i] = ai0 * B.matrix[12] + ai1 * B.matrix[13] + ai2 * B.matrix[14] + ai3 * B.matrix[15];
    }
    return ret;
}

lsVector3 lsMatrix4x4::operator*(const lsVector3& v)
{
    lsVector3 ret;

    ret.x = matrix[ 0]*v.x + matrix[ 1]*v.y + matrix[ 2]*v.z + matrix[ 3];
    ret.y = matrix[ 4]*v.x + matrix[ 5]*v.y + matrix[ 6]*v.z + matrix[ 7];
    ret.z = matrix[ 8]*v.x + matrix[ 9]*v.y + matrix[10]*v.z + matrix[11];

    return ret;
}

//! Rotate transform
/*!
 * Build a rotation matrix.
 * \n\n
 * angle is given in degrees and rotates CCW.\n
 * x,y,z is the vector to rotate around.
 *
 * */
void lsMatrix4x4::RotateMatrix( float angle, float x, float y, float z )
{
    lsMatrix4x4 rot;
    float s, c;
    float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

    angle = angle*(ANG2RAD);
    s = sin(angle);
    c = cos(angle);

    xx = x * x;
    yy = y * y;
    zz = z * z;
    xy = x * y;
    yz = y * z;
    zx = z * x;
    xs = x * s;
    ys = y * s;
    zs = z * s;
    one_c = 1.0 - c;

    rot.matrix[ 0] = (one_c * xx) + c;
    rot.matrix[ 4] = (one_c * xy) - zs;
    rot.matrix[ 8] = (one_c * zx) + ys;
    rot.matrix[12] = 0.0;

    rot.matrix[ 1] = (one_c * xy) + zs;
    rot.matrix[ 5] = (one_c * yy) + c;
    rot.matrix[ 9] = (one_c * yz) - xs;
    rot.matrix[13] = 0.0;

    rot.matrix[ 2] = (one_c * zx) - ys;
    rot.matrix[ 6] = (one_c * yz) + xs;
    rot.matrix[10] = (one_c * zz) + c;
    rot.matrix[14] = 0.0;

    rot.matrix[ 3] = 0.0;
    rot.matrix[ 7] = 0.0;
    rot.matrix[11] = 0.0;
    rot.matrix[15] = 1.0;

    *this *= rot;
}

void lsMatrix4x4::TranslateMatrix(float x, float y, float z)
{
    lsMatrix4x4 trans;

    trans.matrix[12] = x;
    trans.matrix[13] = y;
    trans.matrix[14] = z;

    *this = trans;
}

void lsMatrix4x4::ScaleMatrix(float x, float y, float z)
{
    lsMatrix4x4 mat;

    mat.matrix[ 0] = x;
    mat.matrix[ 5] = y;
    mat.matrix[10] = z;

    *this = mat;
}

void lsMatrix4x4::PerspectiveProjection(float fovy, float aspect_ratio, float near_plane, float far_plane)
{
    matrix[15] = 0;

    float y_scale = Cotangent((fovy*ANG2RAD) / 2 );
    float x_scale = y_scale / aspect_ratio;
    float frustum_length = far_plane - near_plane;

    matrix[ 0] = x_scale;
    matrix[ 5] = y_scale;
    matrix[10] = -((far_plane + near_plane) / frustum_length);
    matrix[11] = -1;
    matrix[14] = -((2 * near_plane * far_plane) / frustum_length );
}

void lsMatrix4x4::OrthographicProjection( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
    LoadIdentity();

    matrix[0] = 2.0 / (xMax - xMin);
    matrix[5] = 2.0 / (yMax - yMin);
    matrix[10] = -2.0 / (zMax - zMin);
    matrix[12] = -((xMax + xMin)/(xMax - xMin));
    matrix[13] = -((yMax + yMin)/(yMax - yMin));
    matrix[14] = -((zMax + zMin)/(zMax - zMin));
    matrix[15] = 1.0;
}


float lsMatrix4x4::Cotangent(float angle)
{
    return (float)(1.0 / tan(angle));
}

//! ViewMatrix
/*!
 * Build a view matrix from the given values.
 * */
void lsMatrix4x4::LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector )
{
    lsMath math;
    lsVector3 e, l, u;
    lsVector3 forward, side, up;
    lsMatrix4x4 rotmat, transmat;

    e = eyePosition;
    l = lookAt;
    u = upVector;

    // make sure up is normalized
    up = upVector;
    up.normalize();
    // up.debug( "up: " );

    // forward vector
    forward = l - e;
    forward.normalize();
    // forward.debug( "forward: " );

    // side vector
    side = math.Cross( forward, up );
    side.normalize();
    // side.debug( "side: " );

    rotmat.matrix[0] = side.x;
    rotmat.matrix[4] = side.y;
    rotmat.matrix[8] = side.z;

    rotmat.matrix[1] = up.x;
    rotmat.matrix[5] = up.y;
    rotmat.matrix[9] = up.z;

    rotmat.matrix[2] = -forward.x;
    rotmat.matrix[6] = -forward.y;
    rotmat.matrix[10] = -forward.z;

    transmat.TranslateMatrix( -e.x, -e.y, -e.z );

    *this *= rotmat * transmat;
}

bool lsMatrix4x4::isEqual(const lsMatrix4x4 &mat)
{
    for( int i=0; i<16; i++ )
    {
        if(( matrix[i] < (mat.matrix[i]-moe) )||( matrix[i] > (mat.matrix[i]+moe) )) return false;
    }
    return true;
}

bool lsMatrix4x4::isIdentity()
{
    // check 1's
    if(( matrix[ 0] < (1.0-moe) )||( matrix[ 0] > (1.0+moe) )) return false;
    if(( matrix[ 5] < (1.0-moe) )||( matrix[ 5] > (1.0+moe) )) return false;
    if(( matrix[10] < (1.0-moe) )||( matrix[10] > (1.0+moe))) return false;
    if(( matrix[15] < (1.0-moe) )||( matrix[15] > (1.0+moe))) return false;

    for( int i=0; i<16; i++ )
    {
        if((i!=0)&&(i!=5)&&(i!=10)&&(i!=15))
        {
            if(( matrix[i] < -moe )||( matrix[i] > moe )) return false;
        }
    }
    return true;
}

void lsMatrix4x4::debug( const string& str )
{
    // For Thread overlap
    stringstream msg;

    msg << "Matrix : " << str << endl;
    int k;
    for( int i=0; i<4; i++ )
    {
        for( int j=0; j<13; j+=4 )
        {
            k = j+i;
            // cout << " m[" << k << "] = " << mat[k];
            msg << "[ " << matrix[k] << " ]   ";

        }
        msg << endl;
    }
    msg << endl;

    cout << msg.str();
}

