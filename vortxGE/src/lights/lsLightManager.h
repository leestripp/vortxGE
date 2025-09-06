#ifndef LSLIGHTMANAGER_H_
#define LSLIGHTMANAGER_H_

// Lighting modes
const int LSLM_MANUAL	=	0;
const int LSLM_AUTO	=	1;

class lsLight;
class lsLightPtr;
class lsScene;
class lsNode;


/*!
 * \brief The lsLightManager class
 *
 * Used by both lsScene and lsObject to manage lighting.\n
 *
 */
class lsLightManager
{
public:
	lsLightManager();
	virtual ~lsLightManager();

	void Add_Light( lsLight* light );
	void Del_Light( lsLightPtr* light );
	void ClearLights();

	void Update();


	lsLightPtr* getLight_list() const
	{
		return light_list;
	}

	lsLight* getDefaultLight() const
	{
		return default_light;
	}

	int getLight_count() const
	{
		return light_count;
	}

private:
	lsLightPtr* light_list;
	lsLight* default_light;
	int light_count;

};

#endif /* LSLIGHTMANAGER_H_ */
