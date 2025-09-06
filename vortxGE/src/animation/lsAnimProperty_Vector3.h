#ifndef LSANIMPROPERTY_VECTOR3_H
#define LSANIMPROPERTY_VECTOR3_H

#include "lsAnimProperty.h"

class lsProperty;
class lsCubicCurve;


class lsAnimProperty_Vector3 : public lsAnimProperty
{
public:
	lsAnimProperty_Vector3( lsProperty *p );
	virtual ~lsAnimProperty_Vector3();

	void Update( double frame );
	void InsertKey(double frame, const lsVector3& val);

private:
	lsCubicCurve *curve1;
	lsCubicCurve *curve2;
	lsCubicCurve *curve3;

};

#endif // LSANIMPROPERTY_VECTOR3_H
