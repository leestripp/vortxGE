#pragma once

#include <vector>

class lsLight;

/**
 * @brief The lsLightManager class
 *
 * Used by lsObject to manage which lights affect it.
 * This manager only stores pointers to lights; it does not own or delete them.
 *
 */
class lsLightManager
{
public:
	lsLightManager();
	virtual ~lsLightManager();
	
	void Add_Light( lsLight* light );
	void Del_Light( lsLight* light );
	void ClearLights();
	
	const std::vector<lsLight*>& getLights() const { return m_light_list; }
	int getLight_count() const { return m_light_list.size(); }

private:
	std::vector<lsLight*> m_light_list;
};
