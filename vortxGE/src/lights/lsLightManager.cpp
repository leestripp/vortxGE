// core
#include "../vortx_debug.h"

// scene
#include "../scene/lsNode.h"
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"

#include "lsLightPtr.h"
#include "lsLightManager.h"


lsLightManager::lsLightManager()
{
	light_list = NULL;
	default_light = NULL;
	light_count = 0;

}

lsLightManager::~lsLightManager()
{
	ClearLights();
}

void lsLightManager::ClearLights()
{
	// Cleanup
	while( light_list )
	{
		Del_Light( light_list );
	}
	light_list = NULL;
	light_count = 0;
}

void lsLightManager::Add_Light( lsLight* light )
{
#if defined(VORTX_DEBUG_LIGHTS)
	cout << "lsLightManager::Add_Light()" << endl;
#endif

	lsLightPtr* ptr;

	ptr = new lsLightPtr();
	if( ptr )
	{
		ptr->setLight( light );

		// add to list
		if( light_list ) light_list->setPrev_lightptr( ptr );
		ptr->setNext_lightptr( light_list );
		light_list = ptr;
		default_light = light;
		light_count++;
	}
}

void lsLightManager::Del_Light( lsLightPtr* light )
{
#if defined(VORTX_DEBUG_LIGHTS)
	cout << "lsLightManager::Del_Light() - Remove light pointer" << endl;
#endif

	if( light->getPrev_lightptr() == 0 )
	{
		// first in list
		light_list = light->getNext_lightptr();
		if( light_list ) light_list->setPrev_lightptr(0);

	} else
	{
		// Close the gap
		light->getPrev_lightptr()->setNext_lightptr( light->getNext_lightptr() );
		if( light->getNext_lightptr() )
		{
			light->getNext_lightptr()->setPrev_lightptr( light->getPrev_lightptr() );
		}
	}
	light_count--;
	delete light;
}



