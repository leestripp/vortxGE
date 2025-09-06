#include "lsKeyFrame.h"

lsKeyFrame::lsKeyFrame()
{
	next_keyframe = NULL;
	prev_keyframe = NULL;
	
	frame = 0;
	value = 0;
	
}

lsKeyFrame::~lsKeyFrame()
{
}


