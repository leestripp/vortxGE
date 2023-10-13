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

#ifndef LSKEYFRAME_H_
#define LSKEYFRAME_H_

#include <iostream>


class lsKeyFrame
{
public:
	lsKeyFrame();
	virtual ~lsKeyFrame();


	lsKeyFrame *getNext_keyframe() const
	{
		return next_keyframe;
	}

	void setNext_keyframe(lsKeyFrame *next_keyframe)
	{
		this->next_keyframe = next_keyframe;
	}

	lsKeyFrame *getPrev_keyframe() const
	{
		return prev_keyframe;
	}

	void setPrev_keyframe(lsKeyFrame *prev_keyframe)
	{
		this->prev_keyframe = prev_keyframe;
	}

	double getFrame() const
	{
		return frame;
	}

	void setFrame(double frame)
	{
		this->frame = frame;
	}

	double getValue() const
	{
		return value;
	}

	void setValue(double value)
	{
		this->value = value;
	}

private:
	lsKeyFrame* next_keyframe;
	lsKeyFrame* prev_keyframe;

	double frame;
	double value;

};

#endif /* LSKEYFRAME_H_ */
