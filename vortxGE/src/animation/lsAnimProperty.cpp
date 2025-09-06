#include "lsAnimProperty.h"


lsAnimProperty::lsAnimProperty( lsProperty * p )
{
	animprop_type = LSANIMPROP_NONE;
	property = p;

	next_animprop = NULL;
	prev_animprop = NULL;
}

lsAnimProperty::~lsAnimProperty()
{
	// self delete
	if( next_animprop ) delete next_animprop;
}

