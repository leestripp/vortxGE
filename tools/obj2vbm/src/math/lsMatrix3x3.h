#ifndef LSMATRIX3X3_H
#define LSMATRIX3X3_H

#include <iostream>
#include <string>

#include "lsMatrix4x4.h"

using namespace std;

class lsMatrix3x3
{
public:
	lsMatrix3x3();
	virtual ~lsMatrix3x3();

	void ExtractRotationMatrix4x4( const lsMatrix4x4& mat );

	// debug
	void debug( const string& str );

	// data
	float matrix[9];

};

#endif // LSMATRIX3X3_H
