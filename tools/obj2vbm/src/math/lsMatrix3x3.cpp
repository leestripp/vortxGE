#include "lsMatrix3x3.h"


lsMatrix3x3::lsMatrix3x3()
{
	// X
	matrix[0] = 1.0;
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	// Y
	matrix[3] = 0.0;
	matrix[4] = 1.0;
	matrix[5] = 0.0;
	// Z
	matrix[6] = 0.0;
	matrix[7] = 0.0;
	matrix[8] = 1.0;
}

lsMatrix3x3::~lsMatrix3x3()
{
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

void lsMatrix3x3::debug( const string& str )
{

	cout << "Matrix : " << str << endl;

	int k;
	for( int i=0; i<3; i++ )
	{
		for( int j=0; j<9; j+=3 )
		{
			k = j+i;
			// cout << " m[" << k << "] = " << mat[k];
			cout << "[ " << matrix[k] << " ]   ";

		}
		cout << endl;
	}
}


