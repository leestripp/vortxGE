/**
 * @file lsProperty.h
 * @brief Base class for animatable properties
 * 
 * lsProperty is the base class for the property system.
 * Properties provide:
 * - Named attributes that can be animated
 * - Type-specific storage (float, vector, etc.)
 * - Serialization support (toString/fromString)
 * 
 * @section property_types Property Types
 * 
 * - LSPROP_BASE: Base type
 * - LSPROP_FLOAT: Single float
 * - LSPROP_INT: Integer
 * - LSPROP_VECTOR2: 2D vector
 * - LSPROP_VECTOR3: 3D vector
 * - LSPROP_VECTOR4: 4D vector
 * 
 * @section property_ids Property IDs
 * 
 * Predefined IDs for common properties:
 * - LSPROPID_POSITION, LSPROPID_ROTATION, LSPROPID_SCALE
 * - LSPROPID_DIFFUSE, LSPROPID_SPECULAR, LSPROPID_AMBIENT
 * - LSPROPID_SHININESS, LSPROPID_GLOW
 * - LSPROPID_CAMERA_FOV, LSPROPID_CAMERA_RATIO, etc.
 * 
 * @section usage Usage
 * @code
 * lsProperty_Vector3* prop = new lsProperty_Vector3(&myVector);
 * prop->setProp_id(LSPROPID_POSITION);
 * prop->setProp_name("Position");
 * animNode->Add_Property(prop);
 * @endcode
 * 
 * @see lsProperty_Float for float properties
 * @see lsProperty_Vector3 for Vector3 properties
 * @see lsAnimNode for animatable nodes
 */

#ifndef LSPROPERTY_H
#define LSPROPERTY_H

#include <iostream>
#include <string>

//=========================================================================
// Property Type Constants
//=========================================================================

/** @brief Base property type */
const int LSPROP_BASE		= 0;
/** @brief Float property */
const int LSPROP_FLOAT		= 1;
/** @brief Integer property */
const int LSPROP_INT		= 2;
/** @brief 2D vector property */
const int LSPROP_VECTOR2	= 3;
/** @brief 3D vector property */
const int LSPROP_VECTOR3	= 4;
/** @brief 4D vector property */
const int LSPROP_VECTOR4	= 5;

//=========================================================================
// Property ID Constants
//=========================================================================

/** @brief No ID / undefined */
const int LSPROPID_NONE         = 0;
/** @brief Position property */
const int LSPROPID_POSITION     = 1;
/** @brief Rotation property */
const int LSPROPID_ROTATION     = 2;
/** @brief Scale property */
const int LSPROPID_SCALE        = 3;
/** @brief Diffuse color property */
const int LSPROPID_DIFFUSE      = 4;
/** @brief Specular color property */
const int LSPROPID_SPECULAR     = 5;
/** @brief Ambient color property */
const int LSPROPID_AMBIENT      = 6;
/** @brief Shininess property */
const int LSPROPID_SHININESS	= 7;
/** @brief Glow property */
const int LSPROPID_GLOW         = 8;
/** @brief Camera FOV property */
const int LSPROPID_CAMERA_FOV   = 9;
/** @brief Camera aspect ratio property */
const int LSPROPID_CAMERA_RATIO = 10;
/** @brief Camera near plane property */
const int LSPROPID_CAMERA_NEAR  = 11;
/** @brief Camera far plane property */
const int LSPROPID_CAMERA_FAR   = 12;
/** @brief Camera ortho cube property */
const int LSPROPID_CAMERA_ORTHOCUBE   = 13;

using namespace std;

/**
 * @class lsProperty
 * @brief Base class for animatable properties
 * 
 * Properties are named, typed attributes that can be
 * animated by the animation system.
 * 
 * @section serialization Serialization
 * 
 * Properties support:
 * - toString(): Convert to string
 * - fromString(): Load from string
 * 
 * @note Subclass to create new property types
 */
class lsProperty
{
public:
    /**
     * @brief Default constructor
     */
	lsProperty();
	
    /**
     * @brief Destructor
     */
	virtual ~lsProperty();

    //=========================================================================
	// Virtual Methods
	//=========================================================================
	
    /**
     * @brief Serialize to string
     * @return String representation
     */
	virtual const string& toString() = 0;
	
    /**
     * @brief Deserialize from string
     * @param str String to parse
     * @return Parsed string
     */
	virtual const string& fromString( const string& str ) = 0;

    //=========================================================================
	// Type
	//=========================================================================
	
    /**
     * @brief Set property type
     * @param val LSPROP_* constant
     */
	void setProp_type( int val )
	{
		prop_type = val;
	}

    /**
     * @brief Get property type
     * @return LSPROP_* constant
     */
	int getProp_type() const
	{
		return prop_type;
	}

    /**
     * @brief Set property ID
     * @param val LSPROPID_* constant
     */
	void setProp_id( int val )
	{
		prop_id = val;
	}

    /**
     * @brief Get property ID
     * @return LSPROPID_* constant
     */
	int getProp_id() const
	{
		return prop_id;
	}

    /**
     * @brief Set property name
     * @param val Name string
     */
	void setProp_name( const string& val )
	{
		name = val;
	}

    /**
     * @brief Get property name
     * @return Name string
     */
	string& getProp_name()
	{
		return name;
	}

    //=========================================================================
	// Navigation
	//=========================================================================
	
    /**
     * @brief Set next property
     * @param val Pointer to next property
     */
	void setNext_prop( lsProperty *val )
	{
		next_prop = val;
	}

    /**
     * @brief Get next property
     * @return Pointer to next property
     */
	lsProperty* getNext_prop() const
	{
		return next_prop;
	}

    /**
     * @brief Set previous property
     * @param val Pointer to previous property
     */
	void setPrev_prop( lsProperty *val )
	{
		prev_prop = val;
	}

    /**
     * @brief Get previous property
     * @return Pointer to previous property
     */
	lsProperty* getPrev_prop() const
	{
		return prev_prop;
	}


private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Next property in list */
	lsProperty *next_prop;
    
    /** @brief Previous property in list */
	lsProperty *prev_prop;

    /** @brief Property type (LSPROP_*) */
	int prop_type;
    
    /** @brief Property ID (LSPROPID_*) */
	int prop_id;
    
    /** @brief Property name */
	string name;

};

#endif // LSPROPERTY_H
