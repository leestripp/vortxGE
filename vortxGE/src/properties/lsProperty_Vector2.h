#ifndef LSPROPERTY_VECTOR2_H
#define LSPROPERTY_VECTOR2_H

#include "lsProperty.h"


class lsVector2;


class lsProperty_Vector2 : public lsProperty
{
public:
	lsProperty_Vector2( lsVector2 *val );
	virtual ~lsProperty_Vector2();

	void Set( float x, float y );

    const string& toString();
	const string& fromString( const string& str );

	float Get_X();
	float Get_Y();

	void Set_X(float val);
	void Set_Y(float val);


private:
	lsVector2 *vec2;

	// toString
	string output;

};

#endif // LSPROPERTY_VECTOR2_H
