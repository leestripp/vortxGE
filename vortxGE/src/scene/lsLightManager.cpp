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
