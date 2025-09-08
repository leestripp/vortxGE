#ifndef LSPROPERTY_FLOAT_H
#define LSPROPERTY_FLOAT_H

#include <sstream>
#include <string>

#include "lsProperty.h"

using namespace std;

//! Float values
/*!
 * Used by the animation system to modigy float values.
 *
 * */
class lsProperty_Float : public lsProperty
{
public:
	lsProperty_Float( float *val );
	virtual ~lsProperty_Float();

	void Set( float x );

    // lsProperty
	const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	void Set_X(float val);

private:
	float *vec1;

	// toString
	string output;

};

#endif // LSPROPERTY_FLOAT_H
