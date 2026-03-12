/**
 * @file lsKeyFrame.h
 * @brief Keyframe data for animation
 * 
 * lsKeyFrame represents a single keyframe in an animation.
 * It stores:
 * - Frame number (time position)
 * - Value at that frame
 * - Links to previous/next keyframes
 * 
 * @section usage Usage
 * @code
 * lsKeyFrame* key = new lsKeyFrame();
 * key->setFrame(30.0);
 * key->setValue(1.0f);
 * @endcode
 * 
 * @see lsCubicCurve for keyframe interpolation
 * @see lsAnimationManager for animation system
 */

#ifndef LSKEYFRAME_H_
#define LSKEYFRAME_H_

#include <iostream>

/**
 * @class lsKeyFrame
 * @brief Single keyframe in an animation curve
 * 
 * Stores a single keyframe value at a specific time.
 * Keyframes are linked in a list for efficient
 * lookup during interpolation.
 */
class lsKeyFrame
{
public:
    /**
     * @brief Default constructor
     */
	lsKeyFrame();
	
    /**
     * @brief Destructor
     */
	virtual ~lsKeyFrame();

    //=========================================================================
	// Navigation
	//=========================================================================
	
    /**
     * @brief Get next keyframe
     * @return Pointer to next keyframe
     */
	lsKeyFrame *getNext_keyframe() const
	{
		return next_keyframe;
	}

    /**
     * @brief Set next keyframe
     * @param next_keyframe Pointer to next
     */
	void setNext_keyframe(lsKeyFrame *next_keyframe)
	{
		this->next_keyframe = next_keyframe;
	}

    /**
     * @brief Get previous keyframe
     * @return Pointer to previous keyframe
     */
	lsKeyFrame *getPrev_keyframe() const
	{
		return prev_keyframe;
	}

    /**
     * @brief Set previous keyframe
     * @param prev_keyframe Pointer to previous
     */
	void setPrev_keyframe(lsKeyFrame *prev_keyframe)
	{
		this->prev_keyframe = prev_keyframe;
	}

    //=========================================================================
	// Keyframe Data
	//=========================================================================
	
    /**
     * @brief Get frame number
     * @return Frame/time position
     */
	double getFrame() const
	{
		return frame;
	}

    /**
     * @brief Set frame number
     * @param frame Frame/time position
     */
	void setFrame(double frame)
	{
		this->frame = frame;
	}

    /**
     * @brief Get value
     * @return Keyframe value
     */
	double getValue() const
	{
		return value;
	}

    /**
     * @brief Set value
     * @param value Keyframe value
     */
	void setValue(double value)
	{
		this->value = value;
	}

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Next keyframe in list */
	lsKeyFrame* next_keyframe;
    
    /** @brief Previous keyframe in list */
	lsKeyFrame* prev_keyframe;

	/** @brief Frame/time position */
	double frame;
    
    /** @brief Value at this frame */
	double value;

};

#endif /* LSKEYFRAME_H_ */
