#ifndef LSANIMATIONMANAGER_H
#define LSANIMATIONMANAGER_H

// timers
#include "../timers/lsTimer.h"
// math
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

class lsAnimNode;
class lsProperty;
class lsAnimProperty;
class lsAnimProperty_Vector3;
class lsAnimProperty_Vector4;
class lsBatch;


//! Anim property set list
/*!
 * Manage a list of animation property sets.
 * Play once, Loop and Stop the property keys.
 *
 * */
class lsAnimationManager
{
public:
	lsAnimationManager();
	virtual ~lsAnimationManager();

    void Init( lsAnimNode *n, double fps );

    void UpdateAll( double delta );
    void Play( bool loop );
    void Stop();

	void Add_AnimProperty( lsProperty *p );

	// Set keys Transform
	void InsertKey_Pos( double frame, const lsVector3& pos );
	void InsertKey_Rot( double frame, const lsVector3& rot );
	void InsertKey_Scale( double frame, const lsVector3& sca );
	// Set keys Material
	void InsertKey_Diffuse( double frame, const lsVector4& val );
	void InsertKey_Specular( double frame, const lsVector4& val );
	void InsertKey_Ambient( double frame, const lsVector4& val );


	void setFramerate( double val )
	{
		framerate = val;
	}


private:
	lsAnimProperty *animproperty_list;
	lsAnimNode *animnode;

	// data
	double framerate;
	double curr_frame;
	// Play range
	double start_frame;
	double end_frame;
	bool looping;
    bool pause;

	// Fast lookups
	// Transforms
	lsAnimProperty_Vector3 *position;
	lsAnimProperty_Vector3 *rotation;
	lsAnimProperty_Vector3 *scale;
	// Batch materials
	lsAnimProperty_Vector4 *diffuse;
	lsAnimProperty_Vector4 *specular;
	lsAnimProperty_Vector4 *ambient;

};

#endif // LSANIMATIONMANAGER_H
