#pragma once

#include <iostream>
#include <string>
#include <vector>

// render
#include "../render/lsRender.h"

// timers
#include "../timers/lsTimer.h"

// bullet
#include <bullet/btBulletDynamicsCommon.h>

// math
#include "../math/lsRandom.h"
#include "../math/lsVector3.h"
#include "../math/lsMatrix4x4.h"

#include "lsAnimNode.h"

class lsAsset_Mesh;
class lsAsset;
class lsObject;
class lsBulletObject;
class lsScene;
class lsLight;

using namespace std;


//! Emits an Object with physics.
/*!
 * Emits a mesh object set by the user.
 * Attach a physics shape to enable collisions.
 *
 * Remember to call Init_Animation() after creating an instance or no animation will be applied.
 *
 */
class lsObjectEmitter : public lsAnimNode
{
public:
	lsObjectEmitter( lsAsset_Mesh *am, lsAsset *abs, lsNode* parent );
	virtual ~lsObjectEmitter();

	// Emit
	void EmitObject( lsScene *scn, lsRender *render );

	// Object pool (redesign 2026-08-04): pre-allocate ALL emitted objects
	// once — hidden, immortal (life==0), physics bodies parked asleep.
	// Spawns just reset an entry's transform/velocity; when the pool is
	// exhausted the OLDEST entry is reused (ring cursor). Nothing is ever
	// created or deleted at spawn time — no VRAM/body churn.
	void PreAllocate( lsScene* scn, lsRender* render );	// idempotent
	void setPoolSize( int n ) { if( !m_allocated && n > 0 ) m_poolSize = n; }
	int getPoolSize() const { return m_poolSize; }
	size_t getSpawnedCount() const { return m_spawned; }

	// Override functions
	virtual void Move( lsScene* scn, lsRender* render, double delta );

	// Get Set
	double getEmittime() const;
	void setEmittime(double value);

	lsAsset_Mesh *getEmit_mesh() const;
	void setEmit_mesh(lsAsset_Mesh *value);

	lsAsset *getEmit_bulletshape() const;
	void setEmit_bulletshape(lsAsset *value);
	
	void setMass(float mass);
	
	double getEmit_life() const;
	void setEmit_life(double value);

	// Light management
	void add_Emit_Light(lsLight* light);
	void remove_Emit_Light(lsLight* light);
	void clear_Emit_Lights();
	const vector<lsLight*>& get_Emit_Lights() const;

	lsVector3 getRand_area() const;
	void setRand_area(const lsVector3 &value);

	lsVector3 getRand_velocity() const;
	void setRand_velocity(const lsVector3 &value);

	bool getRand_rotation() const;
	void setRand_rotation(bool value);

private:
	lsTimer         timer_emit;
	double          emittime;
	
	// Emit data
	lsScene         *m_scene;
	lsAsset_Mesh    *emit_mesh;
	float			m_emit_mass = 1.0f;
	lsAsset         *emit_bulletshape;
	// Life
	double          emit_life;
	
	// Lights for emitted objects
	vector<lsLight*> emit_lights;
	
	// Random ranges
	lsRandom m_random;
	lsVector3   rand_area;
	lsVector3   rand_velocity;
	bool        rand_rotation;

	// Object pool (see PreAllocate)
	struct PoolEntry
	{
		lsObject*		object = nullptr;
		lsBulletObject*	body = nullptr;
	};
	std::vector<PoolEntry>	m_pool;
	int			m_poolSize = 100;
	size_t		m_cursor = 0;		// ring cursor == oldest entry
	size_t		m_spawned = 0;
	bool		m_allocated = false;
	class lsBulletShape* resolveBulletShape( lsRender* render );
};
