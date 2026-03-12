/**
 * @file lsProperty_Float.h
 * @brief Float property for animatable float values
 * 
 * lsProperty_Float provides a property wrapper around
 * a float variable. It allows the animation system
 * to modify the float value over time.
 * 
 * @section usage Usage
 * @code
 * // Create property wrapper
 * lsProperty_Float* prop = new lsProperty_Float(&myFloat);
 * prop->setProp_id(LSPROPID_SHININESS);
 * 
 * // Add to animatable node
 * animNode->Add_Property(prop);
 * 
 * // Animation system can now modify myFloat
 * @endcode
 * 
 * @see lsProperty for base class
 * @see lsAnimProperty for animation integration
 */

#ifndef LSPROPERTY_FLOAT_H
#define LSPROPERTY_FLOAT_H

#include <sstream>
#include <string>

#include "lsProperty.h"

using namespace std;

/**
 * @class lsProperty_Float
 * @brief Float property for animated float values
 * 
 * Wraps a float pointer to allow animation
 * system to modify the value.
 */
class lsProperty_Float : public lsProperty
{
public:
    /**
     * @brief Construct float property
     * @param val Pointer to float to wrap
     */
	lsProperty_Float( float *val );
	
    /**
     * @brief Destructor
     */
	virtual ~lsProperty_Float();

    //=========================================================================
	// Value Access
	//=========================================================================
	
    /**
     * @brief Set float value
     * @param x New value
     */
	void Set( float x );

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
     * @brief Get float value
     * @return Current value
     */
	float Get_X();
	
    /**
     * @brief Set float value
     * @param val New value
     */
	void Set_X(float val);

private:
    /** @brief Wrapped float pointer */
	float *vec1;

    /** @brief Output buffer for toString() */
	string output;

};

#endif // LSPROPERTY_FLOAT_H
