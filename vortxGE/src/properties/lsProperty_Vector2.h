/**
 * @file lsProperty_Vector2.h
 * @brief 2D Vector property for animatable Vector2 values
 * 
 * lsProperty_Vector2 provides a property wrapper around
 * an lsVector2 variable. It allows the animation system
 * to modify the vector components (x, y) over time.
 * 
 * @section usage Usage
 * @code
 * // Create property wrapper
 * lsProperty_Vector2* prop = new lsProperty_Vector2(&myUV);
 * prop->setProp_id(LSPROPID_UV);  // or custom
 * 
 * // Add to animatable node
 * animNode->Add_Property(prop);
 * @endcode
 * 
 * @see lsProperty for base class
 */

#ifndef LSPROPERTY_VECTOR2_H
#define LSPROPERTY_VECTOR2_H

#include "lsProperty.h"

// Forward declaration
class lsVector2;

/**
 * @class lsProperty_Vector2
 * @brief 2D Vector property for animated Vector2 values
 * 
 * Wraps an lsVector2 pointer to allow animation
 * system to modify x, y components.
 */
class lsProperty_Vector2 : public lsProperty
{
public:
    /**
     * @brief Construct Vector2 property
     * @param val Pointer to lsVector2 to wrap
     */
	lsProperty_Vector2( lsVector2 *val );
	
    /**
     * @brief Destructor
     */
	virtual ~lsProperty_Vector2();

    //=========================================================================
	// Value Access
	//=========================================================================
	
    /**
     * @brief Set all components
     * @param x X component
     * @param y Y component
     */
	void Set( float x, float y );

    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Serialize to string
     * @return String representation
     */
    const string& toString();
	
    /**
     * @brief Deserialize from string
     * @param str String to parse
     * @return Parsed string
     */
	const string& fromString( const string& str );

    //=========================================================================
	// Component Access
	//=========================================================================
	
    /**
     * @brief Get X component
     * @return X value
     */
	float Get_X();
	
    /**
     * @brief Get Y component
     * @return Y value
     */
	float Get_Y();

    /**
     * @brief Set X component
     * @param val X value
     */
	void Set_X(float val);
	
    /**
     * @brief Set Y component
     * @param val Y value
     */
	void Set_Y(float val);


private:
    /** @brief Wrapped Vector2 pointer */
	lsVector2 *vec2;

    /** @brief Output buffer for toString() */
	string output;

};

#endif // LSPROPERTY_VECTOR2_H
