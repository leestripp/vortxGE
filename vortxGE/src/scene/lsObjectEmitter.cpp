// Physics
#include "../physics/lsBulletObject.h"
#include "../physics/lsBulletWorld.h"

// Assets
#include "../assets/lsAsset.h"

// Datablocks
#include "../datablocks/lsBulletShape.h"

// Animation
#include "../animation/lsAnimationManager.h"

// Scene
#include "lsScene.h"
#include "lsObject.h"
#include "lsLight.h"
#include "lsLightManager.h"

#include "lsObjectEmitter.h"

lsObjectEmitter::lsObjectEmitter( lsAsset_Mesh *am, lsAsset *abs, lsNode* parent  ) : lsAnimNode(parent)
{
	setNode_type( LSNT_OBJECT_EMITTER );
	setName("Object Emitter");

	// object data to emit
	emit_mesh = am;
	emit_bulletshape = abs;
	// Life
	emit_life = 5000.0;
	// Lights
	emit_lights.clear();

	// randome defaults.
	rand_area = lsVector3( 5, 5, 5 );
	rand_velocity = lsVector3( 0, 0, 0 );
	rand_rotation = true;

	// Emit
	timer_emit.start_Timer();
	emittime = 2000.0;

	// Get the root node
	m_scene = parent->Get_Scene();
}

lsObjectEmitter::~lsObjectEmitter()
{
	// Clean up any remaining lights in our vector
	emit_lights.clear();
}

lsBulletShape* lsObjectEmitter::resolveBulletShape( lsRender* render )
{
	switch( emit_bulletshape->getAsset_type() )
	{
	case LSAT_BULLETSHAPE_BOX:
	case LSAT_BULLETSHAPE_SPHERE:
	case LSAT_BULLETSHAPE_PLANE:
	case LSAT_BULLETSHAPE_PILL:
		return (lsBulletShape*)emit_bulletshape->getData();
	default:
		cout << "Warning: Wrong Asset attached as bullet shape to : " << getName() << ", adding default sphere shape" << endl;
		lsBulletShape* s = new lsBulletShape( render );
		s->Make_Sphere( 1.0f );
		return s;
	}
}

void lsObjectEmitter::PreAllocate( lsScene* scn, lsRender* render )
{
	if( m_allocated ) return;
	m_allocated = true;

	lsBulletShape* bulletshape = resolveBulletShape( render );
	m_pool.reserve( m_poolSize );

	for( int i = 0; i < m_poolSize; i++ )
	{
		lsObject* object = new lsObject( scn );
		if( !object ) break;

		object->Set_Asset( (lsAsset*)emit_mesh );
		// parked far below the world: frustum-culled, physics falls asleep
		object->Set_Pos( 0.0f, -10000.0f, 0.0f );
		object->ModelMatrix.matrix[13] = -10000.0f;
		// life stays 0.0 -> immortal: checkLife() early-outs, the scene
		// NEVER deletes pooled objects

		for( auto& light : emit_lights )
			if( light ) object->getLightManager()->Add_Light( light );

		scn->Add_Node( object );

		lsBulletObject* body = new lsBulletObject(
			object, bulletshape->getShape(),
			scn->getBulletworld()->getMax_proxies() );
		if( body )
		{
			body->Init( m_emit_mass );
			body->Set_Gravity( 0, -9.8*2.0, 0 );
			body->Set_Position( 0.0f, -10000.0f, 0.0f );
			body->Set_Velocity( 0, 0, 0 );
		}
		m_pool.push_back( { object, body } );
	}
	cout << "lsObjectEmitter '" << getName() << "' pre-allocated "
		 << m_pool.size() << " objects" << endl;
}

void lsObjectEmitter::EmitObject( lsScene *scn, lsRender *render )
{
	if( timer_emit.get_ElapsedTime() <= emittime ) return;
	timer_emit.start_Timer();

	if( !m_allocated ) PreAllocate( scn, render );
	if( m_pool.empty() ) return;

	// ring cursor == oldest entry: reuse it when the pool is exhausted
	PoolEntry& e = m_pool[m_cursor];
	m_cursor = ( m_cursor + 1 ) % m_pool.size();
	m_spawned++;
	if( !e.object ) return;

	// Set position at emitter location + random offset
	float ox = position.x + m_random.Rand_FloatRange( -rand_area.x, rand_area.x );
	float oy = position.y + m_random.Rand_FloatRange( -rand_area.y, rand_area.y );
	float oz = position.z + m_random.Rand_FloatRange( -rand_area.z, rand_area.z );
	e.object->Set_Pos( ox, oy, oz );
	// Stop world centre flicker.
	e.object->ModelMatrix.matrix[12] = ox;
	e.object->ModelMatrix.matrix[13] = oy;
	e.object->ModelMatrix.matrix[14] = oz;

	if( e.body )
	{
		// wake and re-place the parked body
		e.body->Set_Position( ox, oy, oz );

		if( rand_rotation )
		{
			float rx = m_random.Rand_FloatRange( 0, 360.0f );
			float ry = m_random.Rand_FloatRange( 0, 360.0f );
			float rz = m_random.Rand_FloatRange( 0, 360.0f );
			e.body->Set_Rotation( rx, ry, rz );
			e.object->Set_Rot( rx, ry, rz );
		}
		float vx = m_random.Rand_FloatRange( -rand_velocity.x, rand_velocity.x );
		float vy = m_random.Rand_FloatRange( -rand_velocity.y, rand_velocity.y );
		float vz = m_random.Rand_FloatRange( -rand_velocity.z, rand_velocity.z );
		e.body->Set_Velocity( vx, vy, vz );
	}
}


void lsObjectEmitter::Move( lsScene *scn, lsRender *render, double delta )
{
	// apply animation
	if( getAnimationManager() ) 
		getAnimationManager()->UpdateAll( delta );
	
	// Emit Object
	EmitObject( scn, render );
	
	// Life
	checkLife( scn );
}

// #################################################

double lsObjectEmitter::getEmittime() const
{
	return emittime;
}

void lsObjectEmitter::setEmittime(double value)
{
	emittime = value;
}

lsAsset_Mesh *lsObjectEmitter::getEmit_mesh() const
{
	return emit_mesh;
}

void lsObjectEmitter::setEmit_mesh(lsAsset_Mesh *value)
{
	emit_mesh = value;
}

lsAsset *lsObjectEmitter::getEmit_bulletshape() const
{
	return emit_bulletshape;
}

void lsObjectEmitter::setEmit_bulletshape(lsAsset *value)
{
	emit_bulletshape = value;
}

void lsObjectEmitter::setMass(float mass)
{
	m_emit_mass = mass;
}

double lsObjectEmitter::getEmit_life() const
{
	return emit_life;
}

void lsObjectEmitter::setEmit_life(double value)
{
	emit_life = value;
}

// Light management methods

void lsObjectEmitter::add_Emit_Light(lsLight* light)
{
	if(! light) return;
	for(const auto l : emit_lights)
	{
		if(l == light)
		{
			return;
		}
	}
	// Add light
	emit_lights.push_back(light);
}

void lsObjectEmitter::remove_Emit_Light(lsLight* light)
{
    if(! light) return;
    for(auto it = emit_lights.begin(); it != emit_lights.end(); ++it)
    {
        if(*it == light)
        {
            emit_lights.erase(it);
            break; // Exit after removing to avoid invalid iterator
        }
    }
}

void lsObjectEmitter::clear_Emit_Lights()
{
	emit_lights.clear();
}

const vector<lsLight*>& lsObjectEmitter::get_Emit_Lights() const
{
	return emit_lights;
}

lsVector3 lsObjectEmitter::getRand_area() const
{
	return rand_area;
}

void lsObjectEmitter::setRand_area(const lsVector3 &value)
{
	rand_area = value;
}

lsVector3 lsObjectEmitter::getRand_velocity() const
{
	return rand_velocity;
}

void lsObjectEmitter::setRand_velocity(const lsVector3 &value)
{
	rand_velocity = value;
}

bool lsObjectEmitter::getRand_rotation() const
{
	return rand_rotation;
}

void lsObjectEmitter::setRand_rotation(bool value)
{
	rand_rotation = value;
}

