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
