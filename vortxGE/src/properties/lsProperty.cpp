#include "lsProperty.h"


lsProperty::lsProperty()
{
	prop_type = LSPROP_BASE;
	prop_id = LSPROPID_NONE;
	name = "Base Property";

	next_prop = NULL;
	prev_prop = NULL;
}

lsProperty::~lsProperty()
{
	// Self cleanup
	if( next_prop ) delete next_prop;
}

