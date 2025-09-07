#ifndef LSMATRIX3X3_H
#define LSMATRIX3X3_H

#include <iostream>
#include <string>

#include "lsMatrix4x4.h"

// array types
typedef float	Matrix33f[9];

class lsVector3; // Forward Declaration

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
	
    void LoadIdentity();
    void Transpose( const lsMatrix3x3& mat );
    void Inverse( const lsMatrix3x3& mat );
    // checks
    bool isEqual( const lsMatrix3x3& mat );
    bool isIdentity();
	
    lsMatrix3x3 operator*( const lsMatrix3x3& B );
    lsVector3 operator*( const lsVector3& v );
	
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
