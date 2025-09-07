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
	// row 0
	matrix[0] = mat.matrix[0];
	matrix[1] = mat.matrix[4];
	matrix[2] = mat.matrix[8];
	// row 1
	matrix[3] = mat.matrix[1];
	matrix[4] = mat.matrix[5];
	matrix[5] = mat.matrix[9];
	// row 2
	matrix[6] = mat.matrix[2];
	matrix[7] = mat.matrix[6];
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

lsMatrix3x3 lsMatrix3x3::operator*(const lsMatrix3x3 &B)
{
	lsMatrix3x3 res;
	lsMatrix3x3 A;
	for( int i=0; i<9; i++ ) A.matrix[i] = matrix[i];
	
    for( int i = 0; i < 3; i++ )
    {
        float ai0 = A.matrix[0*3+i];
        float ai1 = A.matrix[1*3+i];
        float ai2 = A.matrix[2*3+i];
		
		res.matrix[0*3+i] = ai0 * B.matrix[ 0] + ai1 * B.matrix[ 1] + ai2 * B.matrix[ 2];
		res.matrix[1*3+i] = ai0 * B.matrix[ 3] + ai1 * B.matrix[ 4] + ai2 * B.matrix[ 5];
		res.matrix[2*3+i] = ai0 * B.matrix[ 6] + ai1 * B.matrix[ 7] + ai2 * B.matrix[ 8];
    }
    return res;
}

lsVector3 lsMatrix3x3::operator*(const lsVector3& v)
{
    lsVector3 res;
    res.x = matrix[0] * v.x + matrix[1] * v.y + matrix[2] * v.z;
    res.y = matrix[3] * v.x + matrix[4] * v.y + matrix[5] * v.z;
    res.z = matrix[6] * v.x + matrix[7] * v.y + matrix[8] * v.z;
    return res;
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
