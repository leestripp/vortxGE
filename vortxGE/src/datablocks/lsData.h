/**
 * @file lsData.h
 * @brief Base class for data-containing objects
 * 
 * Base class providing common functionality for objects that hold data,
 * including memory usage tracking.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef vortxGE_lsData_h
#define vortxGE_lsData_h

#include <iostream>
#include <string>

using namespace std;

/**
 * @class lsData
 * @brief Base class for data objects
 * 
 * Base class for objects that contain data and need memory tracking.
 * Provides basic memory usage accounting.
 * 
 * @note Used as base for textures, meshes, and other data containers
 */
class lsData
{
public:
	/// Default constructor
    lsData();
    
    /// Destructor
    virtual ~lsData();
    
    //=========================================================================
    // Memory Tracking
    //=========================================================================
    
    /**
     * @brief Get memory used by this object
     * @return Memory in bytes
     */
    unsigned int getMem_used() const
    {
        return mem_used;
    }
    
    /**
     * @brief Set memory used
     * @param val Memory in bytes
     */
    void setMem_used( unsigned int val )
    {
        mem_used = val;
    }
    
    /**
     * @brief Add to memory used
     * @param val Bytes to add
     */
    void addMem_used( unsigned int val )
	{
		mem_used += val;
	}
	
    
private:
    /// Memory used by this object in bytes
    unsigned int mem_used;
    
};


#endif
