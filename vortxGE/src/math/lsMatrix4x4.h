#ifndef LSMATRIX4X4_H_
#define LSMATRIX4X4_H_

#include <iostream>
#include <string>
#include <memory>

// array types
typedef float	Matrix44f[16];

class lsVector3;
class lsQuaternion;

using namespace std;

//! Matrix
/*!
 * Holds a 4x4 Matrix in the OpenGL format
 *
 */
 
class lsMatrix4x4
{
public:
    lsMatrix4x4();
    
    lsMatrix4x4& operator=( const float mat4[16] );
	
    lsMatrix4x4 operator*( const lsMatrix4x4& B );
    lsMatrix4x4 operator*( const float mat4[16] );
    lsVector3 operator*( const lsVector3 &v);
	
	lsVector3 Rotate(const lsVector3 &v);
    void Set(const lsQuaternion &q, float x=0, float y=0, float z=0);
    void LoadIdentity();
	
    // Transformations
    void RotateMatrix( float angle, float x, float y, float z );
    void TranslateMatrix( float x, float y, float z );
    void ScaleMatrix( float x, float y, float z );

    // Utilities
    void PerspectiveProjection( float fovy, float aspect_ratio, float near_plane, float far_plane );
    void OrthographicProjection( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
    float Cotangent(float angle);
    void LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector );
	lsMatrix4x4& transpose( const lsMatrix4x4& m );
    // checks
    bool isEqual( const lsMatrix4x4& mat );
    bool isIdentity();
	
    // Debug
    void debug( const string& str );
	
    // Data
    float matrix[16];
	
private:
    float moe;
};

#endif
