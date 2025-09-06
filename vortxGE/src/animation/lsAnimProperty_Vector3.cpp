// math
#include "../math/lsVector3.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector3.h"

#include "lsAnimProperty_Vector3.h"
#include "lsCubicCurve.h"


lsAnimProperty_Vector3::lsAnimProperty_Vector3( lsProperty * p ) : lsAnimProperty(p)
{
	setAnimprop_type( LSANIMPROP_VECTOR3 );

	// Add 3 curves
	curve1 = new lsCubicCurve();
	curve2 = new lsCubicCurve();
	curve3 = new lsCubicCurve();

}

lsAnimProperty_Vector3::~lsAnimProperty_Vector3()
{
	if( curve1 ) delete curve1;
	if( curve2 ) delete curve2;
	if( curve3 ) delete curve3;
}

void lsAnimProperty_Vector3::Update( double frame )
{
    // No animation
    if( curve1->Empty() && curve2->Empty() && curve3->Empty() ) return;

	lsProperty_Vector3 *pv3;

	pv3 = (lsProperty_Vector3*)getProperty();
	if( pv3 )
    {
		pv3->Set( curve1->Get_Value( frame ), curve2->Get_Value( frame ), curve3->Get_Value( frame ) );
	}
}

void lsAnimProperty_Vector3::InsertKey(double frame, const lsVector3& val)
{
	curve1->Add_Key( frame, val.x );
	curve2->Add_Key( frame, val.y );
	curve3->Add_Key( frame, val.z );

}



