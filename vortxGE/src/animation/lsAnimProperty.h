#ifndef LSANIMPROPERTY_H
#define LSANIMPROPERTY_H

#include <iostream>


const int LSANIMPROP_NONE	= 0;
const int LSANIMPROP_VECTOR2	= 1;
const int LSANIMPROP_VECTOR3	= 2;
const int LSANIMPROP_VECTOR4	= 3;
const int LSANIMPROP_FLOAT	= 4;

class lsProperty;

using namespace std;


//! Animation curve set
/*!
 * Points to a lsProperty and uses its curves to control it.
 *
 * */
class lsAnimProperty
{
public:
	lsAnimProperty( lsProperty * p );
	virtual ~lsAnimProperty();

	virtual void Update( double frame ) = 0;


	void setAnimprop_type( int val )
	{
		animprop_type = val;
	}

	int getAnimprop_type() const
	{
		return animprop_type;
	}

	void setProperty( lsProperty *val )
	{
		property = val;
	}

	lsProperty* getProperty() const
	{
		return property;
	}

	// Navi ###################################################

	void setNext_animprop( lsAnimProperty *val )
	{
		next_animprop = val;
	}

	lsAnimProperty* getNext_animprop() const
	{
		return next_animprop;
	}

	void setPrev_animprop( lsAnimProperty *val )
	{
		prev_animprop = val;
	}

	lsAnimProperty* getPrev_animprop() const
	{
		return prev_animprop;
	}


private:
	lsAnimProperty *next_animprop;
	lsAnimProperty *prev_animprop;

	lsProperty *property;
	int animprop_type;

};

#endif // LSANIMPROPERTY_H

