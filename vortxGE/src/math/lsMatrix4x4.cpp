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

lsMatrix4x4& lsMatrix4x4::operator=( const float mat4[16] )
{
	memcpy( matrix, mat4, sizeof(float) * 16 );
	return *this;
}

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

lsVector3 lsMatrix4x4::Rotate( const lsVector3& v )
{
	lsVector3 ret;
	
    ret.x = matrix[0]*v.x + matrix[4]*v.y + matrix[8]*v.z;
    ret.y = matrix[1]*v.x + matrix[5]*v.y + matrix[9]*v.z;
    ret.z = matrix[2]*v.x + matrix[6]*v.y + matrix[10]*v.z;
	
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

lsMatrix4x4 lsMatrix4x4::operator*( const lsMatrix4x4& B )
{
	lsMatrix4x4 res;
    const float* a = this->matrix;
    const float* b = B.matrix;

    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            res.matrix[j * 4 + i] =
                a[0 * 4 + i] * b[j * 4 + 0] +
                a[1 * 4 + i] * b[j * 4 + 1] +
                a[2 * 4 + i] * b[j * 4 + 2] +
                a[3 * 4 + i] * b[j * 4 + 3];
        }
    }
    return res;
}

lsMatrix4x4 lsMatrix4x4::operator*( const float mat4[16] )
{
	lsMatrix4x4 res;
    const float* a = this->matrix;
    const float* b = mat4;

    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            res.matrix[j * 4 + i] =
                a[0 * 4 + i] * b[j * 4 + 0] +
                a[1 * 4 + i] * b[j * 4 + 1] +
                a[2 * 4 + i] * b[j * 4 + 2] +
                a[3 * 4 + i] * b[j * 4 + 3];
        }
    }
    return res;
}

lsVector3 lsMatrix4x4::operator*( const lsVector3& v )
{
    lsVector3 ret;
	
    ret.x = matrix[0]*v.x + matrix[4]*v.y + matrix[8]*v.z + matrix[12];
    ret.y = matrix[1]*v.x + matrix[5]*v.y + matrix[9]*v.z + matrix[13];
    ret.z = matrix[2]*v.x + matrix[6]*v.y + matrix[10]*v.z + matrix[14];
	
    return ret;
}

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
	
    *this = *this * rot;
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
    memset(matrix, 0, sizeof(float) * 16);

    float y_scale = Cotangent((fovy * ANG2RAD) / 2.0f);
    float x_scale = y_scale / aspect_ratio;
    float frustum_length = far_plane - near_plane;

    matrix[0] = x_scale;
    matrix[5] = y_scale;
    matrix[10] = -((far_plane + near_plane) / frustum_length);
    matrix[11] = -1.0f;
    matrix[14] = -((2.0f * far_plane * near_plane) / frustum_length);
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

void lsMatrix4x4::LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector )
{
    lsMath math;

    // The "forward" vector is the direction from the eye to the target.
    // In a right-handed system, the camera looks down its -Z axis, so the
    // view's Z-axis is the opposite of the forward vector.
    lsVector3 forward = lookAt - eyePosition;
    forward.normalize();

    // The "side" vector is the cross product of the forward vector and the world's up vector.
    // This gives us the view's X-axis.
    lsVector3 side = math.Cross(forward, upVector);
    side.normalize();

    // The "up" vector is re-calculated to be orthogonal to the side and forward vectors.
    // This gives us the view's Y-axis.
    lsVector3 up = math.Cross(side, forward);

    // The view matrix is the inverse of the camera's world transformation.
    // For a rotation/translation matrix, Inverse(T*R) = Inverse(R) * Inverse(T).
    // Inverse(R) is its transpose. Inverse(T) is a translation by -eyePosition.
    // The resulting matrix transforms world coordinates to view coordinates.
    
    LoadIdentity();

    // Set the rotation part (column-major)
    matrix[0] = side.x;
    matrix[4] = side.y;
    matrix[8] = side.z;

    matrix[1] = up.x;
    matrix[5] = up.y;
    matrix[9] = up.z;

    matrix[2] = -forward.x;
    matrix[6] = -forward.y;
    matrix[10] = -forward.z;

    // Set the translation part
    matrix[12] = -math.Dot(side, eyePosition);
    matrix[13] = -math.Dot(up, eyePosition);
    matrix[14] = math.Dot(forward, eyePosition);
}

lsMatrix4x4& lsMatrix4x4::transpose( const lsMatrix4x4& m )
{
	for( int n=0; n<16; n++ )
	{
		int i = n/4;
		int j = n%4;
		matrix[n] = m.matrix[4*j + i];
	}
	return *this;
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
