#ifndef LSANIMPROPERTY_VECTOR4_H
#define LSANIMPROPERTY_VECTOR4_H

#include "lsAnimProperty.h"
// math
#include "../math/lsVector4.h"

class lsProperty;
class lsCubicCurve;


class lsAnimProperty_Vector4 : public lsAnimProperty
{
public:
	lsAnimProperty_Vector4( lsProperty *p );
	virtual ~lsAnimProperty_Vector4();

	void Update( double frame );
	void InsertKey(double frame, const lsVector4& val);

private:
	lsCubicCurve *curve1;
	lsCubicCurve *curve2;
	lsCubicCurve *curve3;
	lsCubicCurve *curve4;

};

#endif // LSANIMPROPERTY_VECTOR4_H
