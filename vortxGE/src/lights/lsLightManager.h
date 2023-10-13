/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

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
