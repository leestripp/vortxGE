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

#ifndef vortxGE_lsScene_h
#define vortxGE_lsScene_h

#include <thread>
#include <mutex>

#include "lsNode.h"

class lsBulletWorld;
class lsEvent;
class lsObject;

using namespace std;

/*!
 * \brief The lsScene class
 *
 * The base node for any scene in vortxGE.\n
 * \n
 * Holds an Event Manager that controls all nodes in this scene. e.g. To delete a scene object just create an Event and the Scene node will take care of it safely.\n
 *
 */
class lsScene : public lsNode
{
public:
    lsScene( float world_size, int maxProxies );
    virtual ~lsScene();

    // from lsNode
    void Move( lsScene* scn, lsOpenGL* opengl, double delta );
    void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );

    // Get the base Scene, every node has this func.
    lsScene *Get_Scene();
    lsEventManager *Get_EventManager();

    void Del_Node( lsNode* node );
    void ClearScene();
    void Add_DrawObject( lsObject* object );

    // Events
    void Add_Event( lsEvent* event );
    lsEvent* Poll_Event();

    // Save data
    virtual const string& toString();


    lsObject* getDraw_list() const
    {
        return draw_list;
    }

    lsNode* getFound_node() const
    {
        return found_node;
    }

    void setFound_node( lsNode* val )
    {
        found_node = val;
    }

    // physics

    lsBulletWorld* getBulletworld() const
    {
        return bulletworld;
    }

    void setBulletworld( lsBulletWorld *val )
    {
        bulletworld = val;
    }

    // Threads
    int getThreads() const
    {
        return number_of_threads;
    }

    void setThreads( int val )
    {
        number_of_threads = val;
    }

private:
    lsObject* draw_list;

    // Events
    lsEventManager *root_eventmanager;
    // physics
    lsBulletWorld   *bulletworld;
    // Found node
    lsNode* found_node;

    // Threads
    int number_of_threads;
    mutex m_mutex;

};


#endif
