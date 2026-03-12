/**
 * @file lsProperty_Vector4.h
 * @brief 4D Vector property for animatable Vector4 values
 * 
 * lsProperty_Vector4 provides a property wrapper around
 * an lsVector4 variable. It allows the animation system
 * to modify the vector components (x, y, z, w) over time.
 * 
 * Commonly used for:
 * - RGBA colors
 * - Homogeneous coordinates
 * 
 * @section usage Usage
 * @code
 * // Create property wrapper
 * lsProperty_Vector4* prop = new lsProperty_Vector4(&myColor);
 * prop->setProp_id(LSPROPID_DIFFUSE);
 * 
 * // Add to animatable node
 * animNode->Add_Property(prop);
 * @endcode
 * 
 * @see lsProperty for base class
 */

#ifndef LSPROPERTY_VECTOR4_H
#define LSPROPERTY_VECTOR4_H

#include "lsProperty.h"

// Forward declaration
class lsVector4;

/**
 * @class lsProperty_Vector4
 * @brief 4D Vector property for animated Vector4 values
 * 
 * Wraps an lsVector4 pointer to allow animation
 * system to modify x, y, z, w components.
 * 
 * @note Used for RGBA colors in materials
 */
class lsProperty_Vector4 : public lsProperty
{
public:
    /**
     * @brief Construct Vector4 property
     * @param val Pointer to lsVector4 to wrap
     */
	lsProperty_Vector4( lsVector4 *val );
	
    /**
     * @brief Destructor
     */
	virtual ~lsProperty_Vector4();

    //=========================================================================
	// Value Access
	//=========================================================================
	
    /**
     * @brief Set all components
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
	void Set( float x, float y, float z, float w );

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
     * @brief Get W component
     * @return W value
     */
	float Get_W();

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
	
    /**
     * @brief Set W component
     * @param val W value
     */
	void Set_W(float val);


private:
    /** @brief Wrapped Vector4 pointer */
	lsVector4 *vec4;

    /** @brief Output buffer for toString() */
	string output;
};

#endif // LSPROPERTY_VECTOR4_H
