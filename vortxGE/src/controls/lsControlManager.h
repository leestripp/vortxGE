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

#ifndef vortxGE_lsControlManager_h
#define vortxGE_lsControlManager_h

#include <iostream>

class lsControl;
class lsOpenGL;

using namespace std;

/*!
 * \brief Manage other control objects including Composite and Viewer.
 *
 * When building your pipeline add you controls in the right order.\n
 * Add a Composite Control first so it becomes the last item rendered.\n
 * Controls are added into the top of the list.\n
 * \n
 * Parent class is vortxGE.\n
 * vortxGE calls the Update() and Render() functions.\n
 * Resize() sould only be called by the user if the GL context has changed e.g. A Window Resize..\n
 * \n
 */
class lsControlManager
{
public:
    lsControlManager( lsOpenGL* opengl );
    virtual ~lsControlManager();

    void Update( double delta );
    void Render();
	void Resize( int x, int y, int width, int height  );

    void Add_Control( lsControl* control );
    void Del_Control( lsControl* control );

    lsControl* getControl_list() const
    {
        return control_list;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);

private:
    lsControl *control_list;

    lsOpenGL *ogl;

};

#endif
