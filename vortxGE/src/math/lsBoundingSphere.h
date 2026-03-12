/**
 * @file lsBoundingSphere.h
 * @brief Bounding sphere class for collision and culling
 * 
 * Simple bounding sphere used for broad-phase collision detection,
 * view frustum culling, and spatial queries.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef LSBOUNDINGSPHERE_H_
#define LSBOUNDINGSPHERE_H_

#include "lsVector3.h"


/**
 * @class lsBoundingSphere
 * @brief Simple bounding sphere for collision detection and culling
 * 
 * A bounding sphere is defined by a center point and radius. It provides
 * a fast, simple way to test for potential collisions and visibility.
 * 
 * Used for:
 * - Broad-phase collision detection (quick elimination of non-colliding objects)
 * - View frustum culling
 * - Ray casting optimization
 * - Physics collision shapes
 * 
 * @note For more accurate collision, use oriented bounding boxes (OBB)
 * or other specialized shapes. The bounding sphere is a conservative
 * approximation - objects may report collision when they don't actually touch.
 * 
 * @example
 * @code
 * lsBoundingSphere sphere;
 * sphere.center = lsVector3(0, 0, 0);
 * sphere.radius = 5.0f;
 * 
 * // Check distance between two spheres
 * float dist = sphere.distance(otherSphere.center);
 * if (dist < sphere.radius + otherSphere.radius) {
 *     // Potential collision
 * }
 * @endcode
 */
class lsBoundingSphere
{
public:
	/// Default constructor - sphere at origin with radius 0
	lsBoundingSphere();
	
	/// Destructor
	virtual ~lsBoundingSphere();

	/**
	 * @brief Calculate distance from sphere center to a point
	 * 
	 * Computes the Euclidean distance from the sphere's center to
	 * a given point in 3D space.
	 * 
	 * @param V1 Point to measure distance to
	 * @return float Distance from sphere center to the point
	 */
	float distance( const lsVector3& V1 );

	/**
	 * @brief Draw the bounding sphere (debug visualization)
	 * 
	 * Renders the bounding sphere using OpenGL for debugging purposes.
	 * Typically shows wireframe or transparent fill.
	 */
	void draw();

	/// Center point of the bounding sphere
	lsVector3	center;
	
	/// Radius of the bounding sphere
	float		radius;

};

#endif /* LSBOUNDINGSPHERE_H_ */
