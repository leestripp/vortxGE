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

#ifndef vortxGE_lsPassManager_h
#define vortxGE_lsPassManager_h

class lsPass;
class lsOpenGL;

using namespace std;

class lsPassManager
{
public:
    lsPassManager( lsOpenGL* opengl );
    virtual ~lsPassManager();

    void Render();
	void Resize( int x, int y, int width, int height  );

    void Add_Pass( lsPass* pass );
    void Del_Pass( lsPass* pass );

    lsPass* getPass_list() const
    {
        return pass_list;
    }

    void setPass_list( lsPass* val )
    {
        pass_list = val;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);

private:
    lsPass      *pass_list;

    lsOpenGL *ogl;

};


#endif
