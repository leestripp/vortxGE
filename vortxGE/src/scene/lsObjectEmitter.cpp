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

void lsObjectEmitter::EmitObject( lsScene *scn, lsRender *render )
{
	lsObject *object;
	lsBulletObject *bulletobject;
	lsBulletShape *bulletshape;
	float ox, oy, oz;
	
	if( timer_emit.get_ElapsedTime() > emittime )
	{
		// reset timer
		timer_emit.start_Timer();
		
		object = new lsObject( scn );
		if( object )
		{
			object->Set_Asset( (lsAsset *)emit_mesh );
			
			// Set position at emitter location + random offset
			ox = position.x + m_random.Rand_FloatRange( -rand_area.x, rand_area.x );
			oy = position.y + m_random.Rand_FloatRange( -rand_area.y, rand_area.y );
			oz = position.z + m_random.Rand_FloatRange( -rand_area.z, rand_area.z );
			object->Set_Pos( ox, oy, oz );
			
			// Set life
			object->Set_Life( emit_life );
			
			// Add lights to emitted object
			for (auto& light : emit_lights)
			{
				if (light) 
				{
					object->getLightManager()->Add_Light(light);
				}
			}

			// Add node to scene - this will be managed by the scene graph
			scn->Add_Node( object );

			// Get our physics shape
			switch( emit_bulletshape->getAsset_type() )
			{
			case LSAT_BULLETSHAPE_BOX:
			case LSAT_BULLETSHAPE_SPHERE:
			case LSAT_BULLETSHAPE_PLANE:
			case LSAT_BULLETSHAPE_PILL:
				bulletshape = (lsBulletShape*)emit_bulletshape->getData();
				break;

			default:
				cout << "Warning: Wrong Asset attached as bullet shape to : " << getName() << ", adding default sphere shape" << endl;
				bulletshape = new lsBulletShape( render );
				bulletshape->Make_Sphere( 1.0f );
				break;
			}

			// Add lsBulletObject
			bulletobject = new lsBulletObject( object, bulletshape->getShape(), scn->getBulletworld()->getMax_proxies() );
			if( bulletobject )
			{
				bulletobject->Init( 1.0f ); // Mass 1.0 = dynamic
				bulletobject->Set_Gravity( 0, -10, 0 );
				// match position
				bulletobject->Set_Position( ox, oy, oz );

				// rotation
				if( rand_rotation )
				{
					float rx = m_random.Rand_FloatRange( 0, 360.0f );
					float ry = m_random.Rand_FloatRange( 0, 360.0f );
					float rz = m_random.Rand_FloatRange( 0, 360.0f );
					bulletobject->Set_Rotation( rx, ry, rz );
					object->Set_Rot( rx, ry, rz );
				}
				// velocity
				float vx = m_random.Rand_FloatRange( -rand_velocity.x, rand_velocity.x );
				float vy = m_random.Rand_FloatRange( -rand_velocity.y, rand_velocity.y );
				float vz = m_random.Rand_FloatRange( -rand_velocity.z, rand_velocity.z );
				bulletobject->Set_Velocity( vx, vy, vz );

			}// if( bulletobject )

		}// if object
	}// if emittime
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
	m_mass = mass;
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

const std::vector<lsLight*>& lsObjectEmitter::get_Emit_Lights() const
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

