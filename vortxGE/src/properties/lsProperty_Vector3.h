/**
 * @file lsProperty_Vector3.h
 * @brief 3D Vector property for animatable Vector3 values
 * 
 * lsProperty_Vector3 provides a property wrapper around
 * an lsVector3 variable. It allows the animation system
 * to modify the vector components (x, y, z) over time.
 * 
 * @section usage Usage
 * @code
 * // Create property wrapper
 * lsProperty_Vector3* prop = new lsProperty_Vector3(&myPosition);
 * prop->setProp_id(LSPROPID_POSITION);
 * 
 * // Add to animatable node
 * animNode->Add_Property(prop);
 * 
 * // Animation system can now modify myPosition
 * @endcode
 * 
 * @see lsProperty for base class
 * @see lsAnimProperty_Vector3 for animation integration
 */

#ifndef LSPROPERTY_VECTOR3_H
#define LSPROPERTY_VECTOR3_H

#include "lsProperty.h"

// Forward declaration
class lsVector3;

/**
 * @class lsProperty_Vector3
 * @brief 3D Vector property for animated Vector3 values
 * 
 * Wraps an lsVector3 pointer to allow animation
 * system to modify x, y, z components.
 */
class lsProperty_Vector3 : public lsProperty
{
public:
    /**
     * @brief Construct Vector3 property
     * @param val Pointer to lsVector3 to wrap
     */
	lsProperty_Vector3( lsVector3 *val );
	
    /**
     * @brief Destructor
     */
	virtual ~lsProperty_Vector3();

    //=========================================================================
	// Value Access
	//=========================================================================
	
    /**
     * @brief Set all components
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
	void Set( float x, float y, float z );

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
     * @brief Get Z component
     * @return Z value
     */
	float Get_Z();

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
	
    /**
     * @brief Set Z component
     * @param val Z value
     */
	void Set_Z(float val);


private:
    /** @brief Wrapped Vector3 pointer */
	lsVector3 *vec3;

    /** @brief Output buffer for toString() */
	string output;
};

#endif // LSPROPERTY_VECTOR3_H
