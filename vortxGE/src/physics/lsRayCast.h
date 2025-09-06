#ifndef LSRAYCAST_H
#define LSRAYCAST_H

#include <iostream>
#include <string>

#include <bullet/btBulletDynamicsCommon.h>


//! Bullet Physics Ray
/*!
 * Cast a ray into the scene, point A to B.
 *
 * */
class lsRayCast
{
    public:
        lsRayCast( btDiscreteDynamicsWorld* dw );
        virtual ~lsRayCast();

        void castRay( const btVector3& from,const btVector3& to );

    private:
        btDiscreteDynamicsWorld* world;

        btVector3 hit_pos;
        btVector3 hit_norm;

};

#endif // LSRAYCAST_H
