/**
 * @file lsEvent_HitPoint.h
 * @brief Event for ray cast hit detection
 * 
 * lsEvent_HitPoint is generated when a ray cast
 * intersects with an object. Contains:
 * - Hit object (node)
 * - Hit position in world space
 * - Surface normal at hit point
 * 
 * @section usage Usage
 * @code
 * // After ray cast, check for hit
 * lsEvent_HitPoint* hitEvent = dynamic_cast<lsEvent_HitPoint*>(event);
 * if (hitEvent && hitEvent->getEvent_type() == LSET_HIT_POINT) {
 *     lsNode* hitNode = hitEvent->getHitNode();
 *     lsVector3 hitPos = hitEvent->getHitPoint();
 *     lsVector3 normal = hitEvent->getHitNormal();
 * }
 * @endcode
 * 
 * @see lsRayCast for ray casting
 * @see lsEvent for base event class
 */

#ifndef LSEVENT_HITPOINT_H
#define LSEVENT_HITPOINT_H

#include "../math/lsVector3.h"
#include "lsEvent.h"

// Forward declaration
class lsNode;

/**
 * @class lsEvent_HitPoint
 * @brief Event containing ray hit information
 * 
 * Generated when a ray intersects an object.
 * Contains all information about the intersection.
 */
class lsEvent_HitPoint : public lsEvent
{
public:
    /**
     * @brief Default constructor
     */
	lsEvent_HitPoint();
	
    /**
     * @brief Destructor
     */
	virtual ~lsEvent_HitPoint();

    //=========================================================================
	// Hit Information
	//=========================================================================
	
    /**
     * @brief Set the hit node
     * @param node Pointer to hit object
     */
	void Set_HitNode( lsNode* node );
	
    /**
     * @brief Set hit position
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
	void Set_HitPoint( float x, float y, float z );
	
    /**
     * @brief Set hit surface normal
     * @param x Normal X
     * @param y Normal Y
     * @param z Normal Z
     */
	void Set_HitNormal( float x, float y, float z );

    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get hit node
     * @return Pointer to hit lsNode
     */
	lsNode* getHitNode() const { return hit_node; }
	
    /**
     * @brief Get hit position
     * @return const reference to hit point vector
     */
	const lsVector3& getHitPoint() const { return hit_point; }
	
    /**
     * @brief Get hit normal
     * @return const reference to normal vector
     */
	const lsVector3& getHitNormal() const { return hit_normal; }

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Hit object */
	lsNode* hit_node;
    
    /** @brief Hit position in world space */
	lsVector3 hit_point;
    
    /** @brief Surface normal at hit point */
	lsVector3 hit_normal;
};

#endif // LSEVENT_HITPOINT_H
