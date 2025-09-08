#ifndef LSPROPERTY_VECTOR3_H
#define LSPROPERTY_VECTOR3_H

#include "lsProperty.h"

class lsVector3;

//! lsVector3 Property
/*!
 * Modify the content of a lsvector3 variable.
 *
 * */
class lsProperty_Vector3 : public lsProperty
{
public:
	lsProperty_Vector3( lsVector3 *val );
	virtual ~lsProperty_Vector3();

	void Set( float x, float y, float z );

    const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	float Get_Y();
	float Get_Z();

	void Set_X(float val);
	void Set_Y(float val);
	void Set_Z(float val);


private:
	lsVector3 *vec3;

	// toString
	string output;
};

#endif // LSPROPERTY_VECTOR3_H
