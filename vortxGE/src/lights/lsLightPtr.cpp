// scene
#include "../scene/lsLight.h"

#include "lsLightPtr.h"

lsLightPtr::lsLightPtr()
{
	next_lightptr = NULL;
	prev_lightptr = NULL;

	light = NULL;

}

lsLightPtr::~lsLightPtr()
{
}


