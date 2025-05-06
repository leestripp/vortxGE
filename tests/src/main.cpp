#include <iostream>

// math
#include "math/lsMatrix4x4.h"


using namespace std;

int main()
{
	cout << "vortxGE tests v0.1" << endl;
	
	// Identity
	lsMatrix4x4 mat;
	// display
	mat.debug( "Identity" );
	
	lsMatrix4x4 mat1, mat2;
	for( int i=0; i<16; i++ ) mat1.matrix[i] = (float)i;
	mat2.transpose( mat1 );
	// display
	mat1.debug( "mat1" );
	mat2.debug( "mat2.transpose( mat1 )" );
	
	//**************
	// operator *
	cout << endl << "operator*" << endl;
	mat1 = mat1 * mat2;
	mat1.debug( "mat1 = mat1 * mat2" );
	
	return 0;
}
