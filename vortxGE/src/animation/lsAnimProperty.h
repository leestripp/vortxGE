/**
 * @file lsAnimProperty.h
 * @brief Base class for animated properties
 * 
 * lsAnimProperty connects an animation curve to a property.
 * It provides:
 * - Links animation curves to target properties
 * - Type-specific animation (Vector3, Vector4, Float)
 * - Update mechanism for applying animated values
 * 
 * @section property_types Property Types
 * 
 * - LSANIMPROP_NONE: No type
 * - LSANIMPROP_VECTOR2: 2D vector
 * - LSANIMPROP_VECTOR3: 3D vector
 * - LSANIMPROP_VECTOR4: 4D vector
 * - LSANIMPROP_FLOAT: Single float
 * 
 * @see lsAnimProperty_Vector3 for Vector3 animation
 * @see lsAnimProperty_Vector4 for Vector4 animation
 * @see lsAnimationManager for animation system
 */

#ifndef LSANIMPROPERTY_H
#define LSANIMPROPERTY_H

#include <iostream>

//=========================================================================
// Animation Property Type Constants
//=========================================================================

/** @brief No type / undefined */
const int LSANIMPROP_NONE	= 0;
/** @brief 2D vector animation */
const int LSANIMPROP_VECTOR2	= 1;
/** @brief 3D vector animation */
const int LSANIMPROP_VECTOR3	= 2;
/** @brief 4D vector animation */
const int LSANIMPROP_VECTOR4	= 3;
/** @brief Float animation */
const int LSANIMPROP_FLOAT	= 4;

// Forward declaration
class lsProperty;

/**
 * @class lsAnimProperty
 * @brief Animated property - links curve to target
 * 
 * Connects an animation curve (lsCubicCurve) to
 * a property (lsProperty) that can be animated.
 * 
 * @section update Update Cycle
 * 
 * Each frame:
 * 1. AnimationManager calls Update(frame)
 * 2. AnimProperty interpolates value from curve
 * 3. AnimProperty applies value to target property
 */
class lsAnimProperty
{
public:
    /**
     * @brief Construct animated property
     * @param p Target property to animate
     */
	lsAnimProperty( lsProperty * p );
	
    /**
     * @brief Destructor
     */
	virtual ~lsAnimProperty();

    /**
     * @brief Update property value
     * 
     * Called each frame to interpolate and apply
     * the animated value to the property.
     * 
     * @param frame Current animation frame
     */
	virtual void Update( double frame ) = 0;

    //=========================================================================
	// Type
	//=========================================================================
	
    /**
     * @brief Set animation type
     * @param val LSANIMPROP_* constant
     */
	void setAnimprop_type( int val )
	{
		animprop_type = val;
	}

    /**
     * @brief Get animation type
     * @return LSANIMPROP_* constant
     */
	int getAnimprop_type() const
	{
		return animprop_type;
	}

    //=========================================================================
	// Property
	//=========================================================================
	
    /**
     * @brief Set target property
     * @param val Pointer to property
     */
	void setProperty( lsProperty *val )
	{
		property = val;
	}

    /**
     * @brief Get target property
     * @return Pointer to property
     */
	lsProperty* getProperty() const
	{
		return property;
	}

    //=========================================================================
	// Navigation
	//=========================================================================
	
    /**
     * @brief Set next animated property
     * @param val Pointer to next
     */
	void setNext_animprop( lsAnimProperty *val )
	{
		next_animprop = val;
	}

    /**
     * @brief Get next animated property
     * @return Pointer to next
     */
	lsAnimProperty* getNext_animprop() const
	{
		return next_animprop;
	}

    /**
     * @brief Set previous animated property
     * @param val Pointer to previous
     */
	void setPrev_animprop( lsAnimProperty *val )
	{
		prev_animprop = val;
	}

    /**
     * @brief Get previous animated property
     * @return Pointer to previous
     */
	lsAnimProperty* getPrev_animprop() const
	{
		return prev_animprop;
	}

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Next animated property in list */
	lsAnimProperty *next_animprop;
    
    /** @brief Previous animated property in list */
	lsAnimProperty *prev_animprop;

    /** @brief Target property to animate */
	lsProperty *property;
    
    /** @brief Animation type */
	int animprop_type;

};

#endif // LSANIMPROPERTY_H
