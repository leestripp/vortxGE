#ifndef LSPROPERTY_VECTOR4_H
#define LSPROPERTY_VECTOR4_H

#include "lsProperty.h"

class lsVector4;

//! lsVector4 Property
/*!
 * Modify the content of a lsvector4 variable.
 *
 * */
class lsProperty_Vector4 : public lsProperty
{
public:
	lsProperty_Vector4( lsVector4 *val );
	virtual ~lsProperty_Vector4();

	void Set( float x, float y, float z, float w );

    const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	float Get_Y();
	float Get_Z();
	float Get_W();

	void Set_X(float val);
	void Set_Y(float val);
	void Set_Z(float val);
	void Set_W(float val);


private:
	lsVector4 *vec4;

	// toString
	string output;
};

#endif // LSPROPERTY_VECTOR4_H
