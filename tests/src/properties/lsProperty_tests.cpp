// Tests for vortxGE/src/properties/lsProperty{,_Float,_Vector3}.{h,cpp}
// (Plan.md Phase 5 follow-up — reflective property set/get)
// Pure CPU — no GL context needed.

#include "doctest/doctest.h"

#include "properties/lsProperty_Float.h"
#include "properties/lsProperty_Vector3.h"
#include "math/lsVector3.h"

using doctest::Approx;

TEST_CASE( "lsProperty_Float: reflective set/get through the bound value" )
{
	float bound = 0.0f;
	lsProperty_Float prop( &bound );

	SUBCASE( "Set writes through to the bound float" )
	{
		prop.Set( 3.5f );
		CHECK( bound == Approx( 3.5f ) );
	}
	SUBCASE( "toString reflects the current value" )
	{
		prop.Set( 3.5f );
		CHECK( prop.toString() == "( 3.5 )" );
	}
	SUBCASE( "fromString round-trip writes through" )
	{
		prop.fromString( "( 7.25 )" );
		CHECK( bound == Approx( 7.25f ) );
		CHECK( prop.toString() == "( 7.25 )" );
	}
	SUBCASE( "metadata accessors (type/id/name)" )
	{
		prop.setProp_type( 42 );
		prop.setProp_id( 7 );
		prop.setProp_name( "Health" );
		CHECK( prop.getProp_type() == 42 );
		CHECK( prop.getProp_id() == 7 );
		CHECK( prop.getProp_name() == "Health" );
	}
}

TEST_CASE( "lsProperty_Vector3: reflective set/get" )
{
	lsVector3 bound( 0.0f, 0.0f, 0.0f );
	lsProperty_Vector3 prop( &bound );

	SUBCASE( "Set writes through + component getters" )
	{
		prop.Set( 1.0f, 2.0f, 3.0f );
		CHECK( bound.x == Approx( 1.0f ) );
		CHECK( bound.y == Approx( 2.0f ) );
		CHECK( bound.z == Approx( 3.0f ) );
		CHECK( prop.Get_X() == Approx( 1.0f ) );
		CHECK( prop.Get_Y() == Approx( 2.0f ) );
		CHECK( prop.Get_Z() == Approx( 3.0f ) );
	}
	SUBCASE( "toString/fromString round-trip" )
	{
		prop.Set( 1.5f, -2.5f, 4.0f );
		const std::string s = prop.toString();
		CHECK( s.front() == '(' );
		CHECK( s.back() == ')' );

		lsVector3 other( 0.0f, 0.0f, 0.0f );
		lsProperty_Vector3 prop2( &other );
		prop2.fromString( s );
		CHECK( other.x == Approx( 1.5f ) );
		CHECK( other.y == Approx( -2.5f ) );
		CHECK( other.z == Approx( 4.0f ) );
	}
}
