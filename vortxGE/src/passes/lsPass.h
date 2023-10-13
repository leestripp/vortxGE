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

#ifndef vortxGE_lsPass_h
#define vortxGE_lsPass_h

#include <iostream>
#include <string>

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

class lsScene;
class lsCamera;
class lsLight;
class lsOpenGL;
class lsProperty;

const int LSPT_NONE             = 0;
const int LSPT_DIRECTRENDER     = 1;
const int LSPT_FBO              = 2;
const int LSPT_SHADOWMAP        = 3;
const int LSPT_OCCLUSION        = 4;

using namespace std;

/**
 * Passes control how scenes update, render and to what surface.
 *
 */
class lsPass
{
public:
    lsPass( lsOpenGL* opengl );
    virtual ~lsPass();

    virtual void Update() = 0;
    virtual void Render() = 0;
	virtual void Resize( int x, int y, int width, int height  ) = 0;

    void Add_Property( lsProperty* p );

    // Save
    virtual const string& toString() = 0;


    lsPass* getNext_pass() const
    {
        return next_pass;
    }

    void setNext_pass( lsPass* val )
    {
        next_pass = val;
    }

    lsPass* getPrev_pass() const
    {
        return prev_pass;
    }

    void setPrev_pass( lsPass* val )
    {
        prev_pass = val;
    }

    lsScene* getScene() const
    {
        return scene;
    }

    void setScene( lsScene* val )
    {
        scene = val;
    }

    const string& getName()
    {
        return name;
    }

    void setName( const string& val )
    {
        name = val;
    }

    int getPass_type() const
    {
        return pass_type;
    }

    void setPass_type( int val )
    {
        pass_type = val;
    }

    lsCamera* getCamera() const
    {
        return camera;
    }

    void setCamera( lsCamera* val )
    {
        camera = val;
    }

    lsCamera* getCull_camera() const
    {
        return cull_camera;
    }

    void setCull_camera( lsCamera* val )
    {
        cull_camera = val;
    }

    lsLight* getLight() const
    {
        return light;
    }

    void setLight( lsLight* val )
    {
        light = val;
    }

    // ********

    // viewport
    const lsVector2& getPosition()
	{
		return position;
	}

	void setPosition( const lsVector2& val )
	{
		position = val;
	}

    const lsVector2& getSize()
    {
        return size;
    }

    void setSize( const lsVector2& val )
    {
        size = val;
    }

    const lsVector4& getBackground_colour()
    {
        return background_colour;
    }

    void setBackground_colour( const lsVector4& val )
    {
        background_colour = val;
    }

    float getVratio() const
    {
        return vratio;
    }

    void setVratio( float val )
    {
        vratio = val;
    }

    // defaults

    lsProperty* getProperty_list() const
    {
        return property_list;
    }

    // Save data

    const string& getSave_string()
    {
        return save_string;
    }

    void setSave_string( const string& val )
    {
        save_string = val;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);

private:
    lsPass  *next_pass;
    lsPass  *prev_pass;

    // opengl
    lsOpenGL *ogl;

    string name;
    int pass_type;

    lsScene *scene;
    lsCamera *camera;
    lsCamera *cull_camera;
    lsLight *light;

	// viewport
	lsVector2	position;
    lsVector2   size;
    float       vratio;
    lsVector4   background_colour;

    // Properties
    lsProperty *property_list;

    // Save
    string save_string;

};


#endif
