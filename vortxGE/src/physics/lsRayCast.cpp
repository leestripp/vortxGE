// scene
#include "../scene/lsNode.h"

// game events
#include "../events/lsEventManager.h"
#include "../events/lsEvent.h"
#include "../events/lsEvent_HitPoint.h"

#include "lsRayCast.h"


lsRayCast::lsRayCast( btDiscreteDynamicsWorld* dw )
{
    world = dw;

}

lsRayCast::~lsRayCast()
{
}

void lsRayCast::castRay( const btVector3& from,const btVector3& to )
{
    btCollisionWorld::ClosestRayResultCallback rayCallback( from, to );

    // Fire rayCallback
    world->rayTest( from, to, rayCallback );

    if( rayCallback.hasHit() )
    {
        const btRigidBody* body = btRigidBody::upcast( rayCallback.m_collisionObject );
        if( body && body->hasContactResponse() )
        {
            hit_pos = rayCallback.m_hitPointWorld;
            hit_norm = rayCallback.m_hitNormalWorld;
            hit_norm.normalize();

            // has lsNode
            if( body->getUserPointer() )
            {
                lsNode* obj = (lsNode*)body->getUserPointer();

                // Add hit event
                if( obj->Get_EventManager() )
                {
                    lsEvent_HitPoint* e = new lsEvent_HitPoint();
                    if( e )
                    {
                        e->setEvent_type( LSET_HIT_POINT );
                        e->Set_HitNode( 0 );
                        e->Set_HitPoint( hit_pos.getX(), hit_pos.getY(), hit_pos.getZ() );
                        e->Set_HitNormal( hit_norm.getX(), hit_norm.getY(), hit_norm.getZ() );

                        obj->Get_EventManager()->Add_Event( e );
                    }
                }
            }
        }
    }
}
