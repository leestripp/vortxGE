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
