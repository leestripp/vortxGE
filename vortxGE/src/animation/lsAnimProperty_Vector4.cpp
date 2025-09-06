// math
#include "../math/lsVector4.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector4.h"

#include "lsAnimProperty_Vector4.h"
#include "lsCubicCurve.h"


lsAnimProperty_Vector4::lsAnimProperty_Vector4( lsProperty * p ) : lsAnimProperty(p)
{
    setAnimprop_type( LSANIMPROP_VECTOR4 );

    // Add 3 curves
    curve1 = new lsCubicCurve();
    curve2 = new lsCubicCurve();
    curve3 = new lsCubicCurve();
    curve4 = new lsCubicCurve();

}

lsAnimProperty_Vector4::~lsAnimProperty_Vector4()
{
    if( curve1 ) delete curve1;
    if( curve2 ) delete curve2;
    if( curve3 ) delete curve3;
    if( curve4 ) delete curve4;

}

void lsAnimProperty_Vector4::Update(double frame)
{
    // No animation
    if( curve1->Empty() && curve2->Empty() && curve3->Empty() && curve4->Empty() ) return;

    lsProperty_Vector4 *pv4;

    pv4 = (lsProperty_Vector4*)getProperty();
    if( pv4 )
    {
        pv4->Set( curve1->Get_Value( frame ), curve2->Get_Value( frame ), curve3->Get_Value( frame ), curve4->Get_Value( frame ) );
    }
}

void lsAnimProperty_Vector4::InsertKey(double frame, const lsVector4& val)
{
    curve1->Add_Key( frame, val.x );
    curve2->Add_Key( frame, val.y );
    curve3->Add_Key( frame, val.z );
    curve4->Add_Key( frame, val.w );
}



