#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>
#include <algorithm> // For std::clamp

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

lsMatrix4x4 lsMatrix4x4::operator*( const lsMatrix4x4& B )
{
    lsMatrix4x4 m;

    m.matrix[0] = matrix[0]*B.matrix[0] + matrix[4]*B.matrix[1] + matrix[8]*B.matrix[2] + matrix[12]*B.matrix[3];
    m.matrix[1] = matrix[1]*B.matrix[0] + matrix[5]*B.matrix[1] + matrix[9]*B.matrix[2] + matrix[13]*B.matrix[3];
    m.matrix[2] = matrix[2]*B.matrix[0] + matrix[6]*B.matrix[1] + matrix[10]*B.matrix[2] + matrix[14]*B.matrix[3];
    m.matrix[3] = matrix[3]*B.matrix[0] + matrix[7]*B.matrix[1] + matrix[11]*B.matrix[2] + matrix[15]*B.matrix[3];

    m.matrix[4] = matrix[0]*B.matrix[4] + matrix[4]*B.matrix[5] + matrix[8]*B.matrix[6] + matrix[12]*B.matrix[7];
    m.matrix[5] = matrix[1]*B.matrix[4] + matrix[5]*B.matrix[5] + matrix[9]*B.matrix[6] + matrix[13]*B.matrix[7];
    m.matrix[6] = matrix[2]*B.matrix[4] + matrix[6]*B.matrix[5] + matrix[10]*B.matrix[6] + matrix[14]*B.matrix[7];
    m.matrix[7] = matrix[3]*B.matrix[4] + matrix[7]*B.matrix[5] + matrix[11]*B.matrix[6] + matrix[15]*B.matrix[7];

    m.matrix[8] = matrix[0]*B.matrix[8] + matrix[4]*B.matrix[9] + matrix[8]*B.matrix[10] + matrix[12]*B.matrix[11];
    m.matrix[9] = matrix[1]*B.matrix[8] + matrix[5]*B.matrix[9] + matrix[9]*B.matrix[10] + matrix[13]*B.matrix[11];
    m.matrix[10] = matrix[2]*B.matrix[8] + matrix[6]*B.matrix[9] + matrix[10]*B.matrix[10] + matrix[14]*B.matrix[11];
    m.matrix[11] = matrix[3]*B.matrix[8] + matrix[7]*B.matrix[9] + matrix[11]*B.matrix[10] + matrix[15]*B.matrix[11];

    m.matrix[12] = matrix[0]*B.matrix[12] + matrix[4]*B.matrix[13] + matrix[8]*B.matrix[14] + matrix[12]*B.matrix[15];
    m.matrix[13] = matrix[1]*B.matrix[12] + matrix[5]*B.matrix[13] + matrix[9]*B.matrix[14] + matrix[13]*B.matrix[15];
    m.matrix[14] = matrix[2]*B.matrix[12] + matrix[6]*B.matrix[13] + matrix[10]*B.matrix[14] + matrix[14]*B.matrix[15];
    m.matrix[15] = matrix[3]*B.matrix[12] + matrix[7]*B.matrix[13] + matrix[11]*B.matrix[14] + matrix[15]*B.matrix[15];

    return m;
}

lsMatrix4x4 lsMatrix4x4::operator*( const float mat4[16] )
{
    lsMatrix4x4 B;
    B = mat4;
    return (*this)*B;
}

lsVector3 lsMatrix4x4::operator*( const lsVector3 &v)
{
    lsVector3 ret;

    ret.x = matrix[0]*v.x + matrix[4]*v.y + matrix[8]*v.z + matrix[12];
    ret.y = matrix[1]*v.x + matrix[5]*v.y + matrix[9]*v.z + matrix[13];
    ret.z = matrix[2]*v.x + matrix[6]*v.y + matrix[10]*v.z + matrix[14];
    
    return ret;
}

void lsMatrix4x4::RotateMatrix( float angle, float x, float y, float z )
{
    lsMatrix4x4 m;
    m.LoadIdentity();

    float rad = angle * ANG2RAD;
	
    // Normalise rotation axis
    float length = sqrt(x*x + y*y + z*z);
    x /= length;
    y /= length;
    z /= length;

    float c = cos(rad);
    float s = sin(rad);
    float oneMinusC = 1.0f - c;

    // Build rotation matrix (column-major)
    m.matrix[0] = x*x*oneMinusC + c;
    m.matrix[4] = x*y*oneMinusC - z*s;
    m.matrix[8] = x*z*oneMinusC + y*s;
    m.matrix[12] = 0.0f;
    
    m.matrix[1] = y*x*oneMinusC + z*s;
    m.matrix[5] = y*y*oneMinusC + c;
    m.matrix[9] = y*z*oneMinusC - x*s;
    m.matrix[13] = 0.0f;

    m.matrix[2] = z*x*oneMinusC - y*s;
    m.matrix[6] = z*y*oneMinusC + x*s;
    m.matrix[10] = z*z*oneMinusC + c;
    m.matrix[14] = 0.0f;
    
    m.matrix[3] = 0.0f;
    m.matrix[7] = 0.0f;
    m.matrix[11] = 0.0f;
    m.matrix[15] = 1.0f;

    *this = (*this) * m;
}

void lsMatrix4x4::TranslateMatrix( float x, float y, float z )
{
    lsMatrix4x4 m;
    m.LoadIdentity();

    m.matrix[12] = x;
    m.matrix[13] = y;
    m.matrix[14] = z;

    *this = (*this) * m;
}

void lsMatrix4x4::ScaleMatrix( float x, float y, float z )
{
    lsMatrix4x4 m;
    m.LoadIdentity();
    
    m.matrix[0] = x;
    m.matrix[5] = y;
    m.matrix[10] = z;

    *this = (*this) * m;
}

void lsMatrix4x4::PerspectiveProjection( float fovy, float aspect_ratio, float near_plane, float far_plane )
{
    float f = Cotangent(fovy / 2.0f);

    matrix[0] = f / aspect_ratio;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (far_plane + near_plane) / (near_plane - far_plane);
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
    matrix[15] = 0.0f;
}

void lsMatrix4x4::OrthographicProjection( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
    matrix[0] = 2.0f / (xMax - xMin);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (yMax - yMin);
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -2.0f / (zMax - zMin);
    matrix[11] = 0.0f;

    matrix[12] = -(xMax + xMin) / (xMax - xMin);
    matrix[13] = -(yMax + yMin) / (yMax - yMin);
    matrix[14] = -(zMax + zMin) / (zMax - zMin);
    matrix[15] = 1.0f;
}

float lsMatrix4x4::Cotangent(float angle)
{
    return 1.0f / tan(angle * ANG2RAD);
}

void lsMatrix4x4::LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector )
{
    lsVector3 zaxis = (eyePosition - lookAt).normalize();
    lsVector3 xaxis = (upVector.Cross(zaxis)).normalize();
    lsVector3 yaxis = zaxis.Cross(xaxis);

    matrix[0] = xaxis.x;
    matrix[1] = yaxis.x;
    matrix[2] = zaxis.x;
    matrix[3] = 0.0f;

    matrix[4] = xaxis.y;
    matrix[5] = yaxis.y;
    matrix[6] = zaxis.y;
    matrix[7] = 0.0f;

    matrix[8] = xaxis.z;
    matrix[9] = yaxis.z;
    matrix[10] = zaxis.z;
    matrix[11] = 0.0f;

    matrix[12] = -xaxis.Dot(eyePosition);
    matrix[13] = -yaxis.Dot(eyePosition);
    matrix[14] = -zaxis.Dot(eyePosition);
    matrix[15] = 1.0f;
}

lsMatrix4x4& lsMatrix4x4::transpose( const lsMatrix4x4& m )
{
    matrix[0] = m.matrix[0]; matrix[4] = m.matrix[1]; matrix[8] = m.matrix[2]; matrix[12] = m.matrix[3];
    matrix[1] = m.matrix[4]; matrix[5] = m.matrix[5]; matrix[9] = m.matrix[6]; matrix[13] = m.matrix[7];
    matrix[2] = m.matrix[8]; matrix[6] = m.matrix[9]; matrix[10] = m.matrix[10]; matrix[14] = m.matrix[11];
    matrix[3] = m.matrix[12]; matrix[7] = m.matrix[13]; matrix[11] = m.matrix[14]; matrix[15] = m.matrix[15];
    return *this;
}

// Extracts the 3x3 rotation component from the 4x4 matrix
lsMatrix4x4 lsMatrix4x4::extractRotation()
{
    lsMatrix4x4 rotationMatrix;
    rotationMatrix.LoadIdentity();

    // Copy the top-left 3x3 block (rotation and scale)
    rotationMatrix.matrix[0] = this->matrix[0];
    rotationMatrix.matrix[1] = this->matrix[1];
    rotationMatrix.matrix[2] = this->matrix[2];

    rotationMatrix.matrix[4] = this->matrix[4];
    rotationMatrix.matrix[5] = this->matrix[5];
    rotationMatrix.matrix[6] = this->matrix[6];

    rotationMatrix.matrix[8] = this->matrix[8];
    rotationMatrix.matrix[9] = this->matrix[9];
    rotationMatrix.matrix[10] = this->matrix[10];

    // Set translation components to zero
    rotationMatrix.matrix[12] = 0.0f;
    rotationMatrix.matrix[13] = 0.0f;
    rotationMatrix.matrix[14] = 0.0f;
    
    return rotationMatrix;
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
        if( (i == 0) || (i == 5) || (i == 10) || (i == 15) ) continue;
        if(( matrix[i] < (0.0-moe) )||( matrix[i] > (0.0+moe) )) return false;
    }

    return true;
}

void lsMatrix4x4::Set(const lsQuaternion &q, float x, float y, float z)
{
    // Set position
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;

    // Set rotation
    matrix[0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    matrix[1] = 2.0f * (q.x * q.y + q.z * q.w);
    matrix[2] = 2.0f * (q.x * q.z - q.y * q.w);
    matrix[3] = 0.0f;

    matrix[4] = 2.0f * (q.x * q.y - q.z * q.w);
    matrix[5] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    matrix[6] = 2.0f * (q.y * q.z + q.x * q.w);
    matrix[7] = 0.0f;

    matrix[8] = 2.0f * (q.x * q.z + q.y * q.w);
    matrix[9] = 2.0f * (q.y * q.z - q.x * q.w);
    matrix[10] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    matrix[11] = 0.0f;

    matrix[15] = 1.0f;
}

lsVector3 lsMatrix4x4::ToEulerZYX(const lsMatrix4x4& m)
{
    float roll_rad, pitch_rad, yaw_rad;
    
    // Column-major matrix access: m.matrix[col_idx * 4 + row_idx]
    
    // Extract pitch (rotation.y) from m[2][0] = -sin(pitch)
    // m.matrix[2] corresponds to M[2][0] in row-major notation
    pitch_rad = asin(std::clamp(-m.matrix[2], -1.0f, 1.0f)); 
    
    // Check for gimbal lock (pitch near +/- 90 degrees)
    const float C_EPSILON = 1e-4f; // A small epsilon to check for near-zero cos(pitch)
    if (fabs(cos(pitch_rad)) > C_EPSILON) 
    {
        // General case: No gimbal lock
        // yaw (rotation.z) from m[1][0] and m[0][0]
        // m.matrix[1] corresponds to M[1][0]
        // m.matrix[0] corresponds to M[0][0]
        yaw_rad = atan2(m.matrix[1], m.matrix[0]);
        
        // roll (rotation.x) from m[2][1] and m[2][2]
        // m.matrix[6] corresponds to M[2][1]
        // m.matrix[10] corresponds to M[2][2]
        roll_rad = atan2(m.matrix[6], m.matrix[10]);
    }
    else 
    {
        // Gimbal lock: pitch is +/- 90 degrees
        // In this case, yaw and roll become coupled. We set one to zero (e.g., yaw)
        // and determine the other.
        // For simplicity, set yaw_rad to 0.0f
        yaw_rad = 0.0f; // This is an arbitrary choice.

        // Then calculate roll_rad
        // m.matrix[4] corresponds to M[0][1]
        // m.matrix[5] corresponds to M[1][1]
        roll_rad = atan2(-m.matrix[4], m.matrix[5]);
    }
    
    // Convert radians to degrees for lsVector3 and return
    return lsVector3(roll_rad * RAD2ANG, pitch_rad * RAD2ANG, yaw_rad * RAD2ANG);
}

lsMatrix4x4 lsMatrix4x4::getLocalTransformFromParent(const lsMatrix4x4& parentMatrix) const
{
    // To get the local transform from a parent matrix, we compute:
    // local_transform = parent_inverse * world_transform
    
    // Create a copy of the current (world) matrix
    lsMatrix4x4 worldTransform = *this;
    
    // Create inverse of the parent matrix
    lsMatrix4x4 parentInverse;
    parentInverse.LoadIdentity();
    
    // For a standard transformation matrix, we can compute the inverse more efficiently
    // by separating rotation and translation components
    
    // Extract rotation part from parent (3x3 upper-left)
    lsMatrix4x4 parentRotationOnly;
    parentRotationOnly.LoadIdentity();
    parentRotationOnly.matrix[0] = parentMatrix.matrix[0];
    parentRotationOnly.matrix[1] = parentMatrix.matrix[1];
    parentRotationOnly.matrix[2] = parentMatrix.matrix[2];
    parentRotationOnly.matrix[4] = parentMatrix.matrix[4];
    parentRotationOnly.matrix[5] = parentMatrix.matrix[5];
    parentRotationOnly.matrix[6] = parentMatrix.matrix[6];
    parentRotationOnly.matrix[8] = parentMatrix.matrix[8];
    parentRotationOnly.matrix[9] = parentMatrix.matrix[9];
    parentRotationOnly.matrix[10] = parentMatrix.matrix[10];
    
    // Compute inverse of rotation matrix (transpose for orthonormal matrix)
    parentRotationOnly.transpose(parentRotationOnly);
    
    // Extract translation part from parent
    lsVector3 parentTranslation(parentMatrix.matrix[12], parentMatrix.matrix[13], parentMatrix.matrix[14]);
    
    // Apply inverse rotation to the translation (this is the negative of the world position in local space)
    lsVector3 invertedTranslation = parentRotationOnly.Rotate(parentTranslation);
    invertedTranslation.x = -invertedTranslation.x;
    invertedTranslation.y = -invertedTranslation.y;
    invertedTranslation.z = -invertedTranslation.z;
    
    // Set up inverse matrix with rotated and negated translation
    parentInverse.matrix[0] = parentRotationOnly.matrix[0];
    parentInverse.matrix[1] = parentRotationOnly.matrix[1];
    parentInverse.matrix[2] = parentRotationOnly.matrix[2];
    parentInverse.matrix[4] = parentRotationOnly.matrix[4];
    parentInverse.matrix[5] = parentRotationOnly.matrix[5];
    parentInverse.matrix[6] = parentRotationOnly.matrix[6];
    parentInverse.matrix[8] = parentRotationOnly.matrix[8];
    parentInverse.matrix[9] = parentRotationOnly.matrix[9];
    parentInverse.matrix[10] = parentRotationOnly.matrix[10];
    parentInverse.matrix[12] = invertedTranslation.x;
    parentInverse.matrix[13] = invertedTranslation.y;
    parentInverse.matrix[14] = invertedTranslation.z;
    
    // Compute local transform: parent_inverse * world_transform
    return parentInverse * worldTransform;
}

void lsMatrix4x4::debug( const string& str )
{
    // For Thread overlap
    stringstream msg;
	
    msg << "Matrix : " << str << endl;
	
	int k;
	for( int i=0; i<4; i++ )
	{
		for( int j=0; j<16; j+=4 )
		{
			k = j+i;
			// cout << " m[" << k << "] = " << mat[k];
            msg << "[ " << matrix[k] << " ]   ";
			
		}
        msg << endl;
	}
    cout << msg.str();
}
