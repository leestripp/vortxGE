#include <algorithm>
// core
#include "../vortx_debug.h"

// scene
#include "../scene/lsLight.h"

#include "lsLightManager.h"


lsLightManager::lsLightManager()
{
}

lsLightManager::~lsLightManager()
{
	// Do not delete lights as they are owned by the scene.
	ClearLights();
}

void lsLightManager::ClearLights()
{
	m_light_list.clear();
}

void lsLightManager::Add_Light( lsLight* light )
{
#if defined(VORTX_DEBUG_LIGHTS)
	cout << "lsLightManager::Add_Light()" << endl;
#endif
    if (light) {
	    m_light_list.push_back(light);
    }
}

void lsLightManager::Del_Light( lsLight* light )
{
#if defined(VORTX_DEBUG_LIGHTS)
	cout << "lsLightManager::Del_Light() - Remove light pointer" << endl;
#endif
    auto it = remove(m_light_list.begin(), m_light_list.end(), light);
    if (it != m_light_list.end()) {
        m_light_list.erase(it, m_light_list.end());
    }
}

void lsLightManager::setActiveLights(lsCamera* camera, float maxDistance)
{
    if (! camera || maxDistance <= 0.0f)
    {
        return;
    }
    
    // Get camera position
    lsVector3 camPos(
        camera->ModelMatrix.matrix[12],
        camera->ModelMatrix.matrix[13],
        camera->ModelMatrix.matrix[14]
    );
    
    // Only consider lights within maxDistance
    for(lsLight* light : m_light_list)
    {
        // make direct lights (sun) active
        if(light->getLight_type() == LSLT_DIRECT)
        {
            light->setActive(true);
            continue;
        }
        
        // Get light position
        lsVector3 lightPos(
            light->ModelMatrix.matrix[12],
            light->ModelMatrix.matrix[13],
            light->ModelMatrix.matrix[14]
        );
        
        // Calculate distance
        float distance = camPos.distance(lightPos);
        
        // Add light if within range
        if(distance <= maxDistance)
        {
            light->setActive(true);
        } else
        {
        		 light->setActive(false);
        }
    }
}
