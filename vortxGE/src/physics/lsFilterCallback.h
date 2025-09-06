#ifndef LSFILTERCALLBACK_H
#define LSFILTERCALLBACK_H

#include <bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>

/**
 vortxGE custom bullet physics callback class
 */
class lsFilterCallback : public btOverlapFilterCallback
{
public:
	lsFilterCallback();
	virtual ~lsFilterCallback();

	bool needBroadphaseCollision( btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1 ) const;

private:

};

#endif // LSFILTERCALLBACK_H
