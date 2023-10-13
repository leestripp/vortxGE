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

#ifndef vortxGE_lsAnimNode_h
#define vortxGE_lsAnimNode_h

// prototypes
class lsAnimationManager;
class lsProperty;

#include "lsNode.h"

/** \brief Includes lsAnimationManager
 *
 * Subclass this node to create your own animation class.\n
 *
 */
class lsAnimNode : public lsNode
{
public:
    lsAnimNode( lsNode* p );
    virtual ~lsAnimNode();

	// Override functions
	virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
	virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
	virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    // Scene
    virtual lsScene *Get_Scene();
    virtual lsEventManager *Get_EventManager();

	// animation
    void Init_Animation( double fps );
    void Play_Animation( bool loop );
    void Stop_Animation();

	// Properties
	void Add_Property( lsProperty *p );

    // Save data
    virtual const string& toString();

    lsAnimationManager* getAnimationManager() const
    {
        return animationmanager;
    }

    lsProperty* getProperty_list() const
    {
        return property_list;
    }

private:
    lsAnimationManager  *animationmanager;

    // Properties
    lsProperty          *property_list;

};


#endif
