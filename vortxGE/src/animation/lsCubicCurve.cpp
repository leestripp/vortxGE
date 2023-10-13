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

#include "lsKeyFrame.h"
#include "lsCubicCurve.h"


lsCubicCurve::lsCubicCurve()
{
	keyframe_list = 0;

	keyframe_count = 0;
	currpos = 0.0;

	key0 = NULL;
	key1 = NULL;
	key2 = NULL;
	key3 = NULL;

}

lsCubicCurve::~lsCubicCurve()
{
	Clear_AllKeys();
}

void lsCubicCurve::Clear_AllKeys()
{
	while( keyframe_list )
	{
		Del_Key( keyframe_list );
	}

	keyframe_list = NULL;
	keyframe_count = 0;
	currpos = 0.0;

	key0 = NULL;
	key1 = NULL;
	key2 = NULL;
	key3 = NULL;

}

void lsCubicCurve::InsertKey_Before( lsKeyFrame *key, lsKeyFrame *newkey )
{
	if( key->getPrev_keyframe() == 0 )
	{
		Add_ToList( newkey );
	} else
	{
		// Insert before key
		newkey->setPrev_keyframe( key->getPrev_keyframe() );
		key->getPrev_keyframe()->setNext_keyframe( newkey );
		newkey->setNext_keyframe( key );
		key->setPrev_keyframe( newkey );
	}
}

void lsCubicCurve::Add_ToList( lsKeyFrame *key )
{
	if( keyframe_list ) keyframe_list->setPrev_keyframe( key );
	key->setNext_keyframe( keyframe_list );
	keyframe_list = key;
}


double lsCubicCurve::Get_Value( double frame )
{
	double adjust_frame = 0.0;
	double stepsize = 0.0;
	double rval = 0.0;

	// Need 4+ keys to create a curve
	if( keyframe_count < 4 )
	{
		// Check for first key
		// and return its value
		if( keyframe_list )
		{
			return keyframe_list->getValue();
		} else return 0;
	}

	// check out of current range
	if(( key1 )&&( key2 ))
	{
		if(( frame < key1->getFrame() )||( frame > key2->getFrame() )) key0 = NULL;
	}

	if((! key0 )||(! key1 )||(! key2 )||(! key3 ))
	{
		key0 = Find_Key( frame );
		key1 = key0->getNext_keyframe();
		key2 = key1->getNext_keyframe();
		key3 = key2->getNext_keyframe();
	}
	// Calc position
	stepsize = 1.0 / ( key2->getFrame() - key1->getFrame() );
	adjust_frame = frame - key1->getFrame();
	currpos = stepsize * adjust_frame;
	if( currpos < 0.0 )
	{
		key0 = keyframe_list;
		key1 = key0->getNext_keyframe();
		key2 = key1->getNext_keyframe();
		key3 = key2->getNext_keyframe();
		currpos = 0.0;
	}

	if( key3 )
	{
		rval = catmullRomSpline( currpos, key0->getValue(), key1->getValue(), key2->getValue(), key3->getValue() );
		return rval;
	} else
	{
		return key2->getValue();
	}
}

double lsCubicCurve::catmullRomSpline( double x, double v0, double v1, double v2, double v3 )
{
	double c1,c2,c3,c4;

	c1 = M12*v1;
	c2 = M21*v0 + M23*v2;
	c3 = M31*v0 + M32*v1 + M33*v2 + M34*v3;
	c4 = M41*v0 + M42*v1 + M43*v2 + M44*v3;

	return( ((c4*x + c3)*x +c2)*x + c1 );
}


//! returns key0
/*!
 *
 * */
lsKeyFrame* lsCubicCurve::Find_Key( double frame )
{
	lsKeyFrame *k1;
	lsKeyFrame *k2;
	lsKeyFrame *k3;
	lsKeyFrame *rkey;

	// start on key2
	rkey = keyframe_list;
	k1 = rkey->getNext_keyframe();
	k2 = k1->getNext_keyframe();
	k3 = k2->getNext_keyframe();
	while( k3 )
	{
		if(( frame >= k1->getFrame() )&&( frame < k2->getFrame() )) break;

		rkey = rkey->getNext_keyframe();
		k1 = rkey->getNext_keyframe();
		k2 = k1->getNext_keyframe();
		k3 = k2->getNext_keyframe();
	}

    return rkey;
}

bool lsCubicCurve::Empty()
{
    if( keyframe_list ) return false;
    return true;
}


void lsCubicCurve::Add_Key( double frame, double value )
{
	lsKeyFrame* key;
	lsKeyFrame* newkey;
	lsKeyFrame* prevkey = 0;

	newkey = new lsKeyFrame();
	if( newkey )
	{
		newkey->setFrame( frame );
		newkey->setValue( value );
		keyframe_count++;

#ifdef VORTX_DEBUG_ANIMATION
		cout << "lsCubicCurve::Add_Key() - Key Added - Frame: " << frame << " Value: " << value << endl;
#endif //VORTX_DEBUG_ANIMATION

	} else
	{
		cout << "lsCubicCurve::Add_Key() - ERROR: Cant create Key" << endl;
		return;
	}

	// find insert point
	key = keyframe_list;
	while( key )
	{
		if( frame < key->getFrame() ) break;
		prevkey = key;
		key = key->getNext_keyframe();
	}

	if( key )
	{
		InsertKey_Before( key, newkey );

	} else
	{
		if( prevkey )
		{
			// add to end
			prevkey->setNext_keyframe( newkey );
			newkey->setPrev_keyframe( prevkey );

		} else
		{
			// add to top
			Add_ToList( newkey );
		}
	}
}

void lsCubicCurve::Del_Key( lsKeyFrame* key )
{
	// cout << "lsCubicCurve::Del_Key()" << endl;

	if( key->getPrev_keyframe() == 0 )
	{
		// Top of list
		keyframe_list = key->getNext_keyframe();
		if( keyframe_list ) keyframe_list->setPrev_keyframe(0);

	} else
	{
		// Cloase gap
		key->getPrev_keyframe()->setNext_keyframe( key->getNext_keyframe() );
		if( key->getNext_keyframe() )
		{
			key->getNext_keyframe()->setPrev_keyframe( key->getPrev_keyframe() );
		}
	}
	keyframe_count--;
	delete key;
}



