#include <iostream>
#include <sstream>

#include "lsMatrix3x3.h"
#include "lsVector3.h"


lsMatrix3x3::lsMatrix3x3()
{
    // float isn't accurate so wee need this for tests.
    moe = 0.00001;

    LoadIdentity();
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

void lsMatrix3x3::ExtractRotationMatrix4x4( const lsMatrix4x4& mat )
{
	// Extracts the top-left 3x3 part of a 4x4 column-major matrix
	matrix[0] = mat.matrix[0];
	matrix[1] = mat.matrix[1];
	matrix[2] = mat.matrix[2];

	matrix[3] = mat.matrix[4];
	matrix[4] = mat.matrix[5];
	matrix[5] = mat.matrix[6];

	matrix[6] = mat.matrix[8];
	matrix[7] = mat.matrix[9];
	matrix[8] = mat.matrix[10];
}

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

void lsMatrix3x3::Inverse( const lsMatrix3x3 &mat )
{
    float det = mat.matrix[0] * (mat.matrix[4] * mat.matrix[8] - mat.matrix[7] * mat.matrix[5]) -
                mat.matrix[3] * (mat.matrix[1] * mat.matrix[8] - mat.matrix[7] * mat.matrix[2]) +
                mat.matrix[6] * (mat.matrix[1] * mat.matrix[5] - mat.matrix[4] * mat.matrix[2]);

    if (fabs(det) < 1e-6) {
        LoadIdentity();
        return;
    }

    float invDet = 1.0f / det;

    matrix[0] = (mat.matrix[4] * mat.matrix[8] - mat.matrix[7] * mat.matrix[5]) * invDet;
    matrix[3] = (mat.matrix[6] * mat.matrix[5] - mat.matrix[3] * mat.matrix[8]) * invDet;
    matrix[6] = (mat.matrix[3] * mat.matrix[7] - mat.matrix[6] * mat.matrix[4]) * invDet;
    matrix[1] = (mat.matrix[7] * mat.matrix[2] - mat.matrix[1] * mat.matrix[8]) * invDet;
    matrix[4] = (mat.matrix[0] * mat.matrix[8] - mat.matrix[6] * mat.matrix[2]) * invDet;
    matrix[7] = (mat.matrix[6] * mat.matrix[1] - mat.matrix[0] * mat.matrix[7]) * invDet;
    matrix[2] = (mat.matrix[1] * mat.matrix[5] - mat.matrix[4] * mat.matrix[2]) * invDet;
    matrix[5] = (mat.matrix[3] * mat.matrix[2] - mat.matrix[0] * mat.matrix[5]) * invDet;
    matrix[8] = (mat.matrix[0] * mat.matrix[4] - mat.matrix[3] * mat.matrix[1]) * invDet;
}

lsVector3 lsMatrix3x3::operator*(const lsVector3& v)
{
    lsVector3 res;
    res.x = matrix[0] * v.x + matrix[3] * v.y + matrix[6] * v.z;
    res.y = matrix[1] * v.x + matrix[4] * v.y + matrix[7] * v.z;
    res.z = matrix[2] * v.x + matrix[5] * v.y + matrix[8] * v.z;
    return res;
}

lsMatrix3x3 lsMatrix3x3::operator*(const lsMatrix3x3 &B)
{
	lsMatrix3x3 res;
    const float* a = this->matrix;
    const float* b = B.matrix;

    for (int j = 0; j < 3; ++j) { // B column
        for (int i = 0; i < 3; ++i) { // A row
            float sum = 0.0f;
            for (int k = 0; k < 3; ++k) {
                sum += a[i + k * 3] * b[k + j * 3];
            }
            res.matrix[i + j * 3] = sum;
        }
    }
    return res;
}

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
