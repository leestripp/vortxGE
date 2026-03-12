/**
 * @file lsPlane.h
 * @brief Plane mathematics class for 3D space
 * 
 * Represents a plane in 3D space defined by a normal vector and a point.
 * Used for collision detection, culling, and geometric calculations.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef LSPLANE_H_
#define LSPLANE_H_

#include <iostream>

#include "lsVector3.h"

using namespace std;

/**
 * @class lsPlane
 * @brief Plane math class for 3D space
 * 
 * Represents a mathematical plane in 3D space defined by:
 * - A normal vector (perpendicular direction)
 * - A point on the plane
 * - A distance coefficient (d)
 * 
 * The plane equation is: ax + by + cz + d = 0
 * where (a,b,c) is the normal vector.
 * 
 * @note The plane is infinite in extent - it defines the mathematical
 * plane rather than a bounded surface.
 * 
 * @example
 * @code
 * lsVector3 p1(0, 0, 0);
 * lsVector3 p2(1, 0, 0);
 * lsVector3 p3(0, 1, 0);
 * lsPlane plane(p1, p2, p3);
 * 
 * lsVector3 testPoint(0.5, 0.5, 1.0);
 * float dist = plane.distance(testPoint);
 * @endcode
 */
class lsPlane
{
public:
	/// Default constructor - creates a plane at origin with upward normal
	lsPlane();
	
	/**
	 * @brief Construct plane from three points
	 * 
	 * Creates a plane passing through three non-collinear points.
	 * The normal is computed from the cross product of the edge vectors.
	 * 
	 * @param v1 First point on the plane
	 * @param v2 Second point on the plane
	 * @param v3 Third point on the plane
	 */
	lsPlane( lsVector3 &v1, lsVector3 &v2, lsVector3 &v3 );
	
	/// Destructor
	virtual ~lsPlane();


	/**
	 * @brief Set plane from three points
	 * 
	 * Defines the plane passing through three non-collinear points.
	 * 
	 * @param v1 First point on the plane
	 * @param v2 Second point on the plane
	 * @param v3 Third point on the plane
	 */
	void set3Points( lsVector3 &v1,  lsVector3 &v2,  lsVector3 &v3);
	
	/**
	 * @brief Set plane from normal and a point
	 * 
	 * Defines the plane using a normal vector and any point that lies
	 * on the plane.
	 * 
	 * @param normal The plane's normal vector (must be normalized)
	 * @param point Any point on the plane
	 */
	void setNormalAndPoint( const lsVector3 &normal, const lsVector3 &point);
	
	/**
	 * @brief Set plane from coefficients
	 * 
	 * Sets the plane using the coefficients of the plane equation:
	 * ax + by + cz + d = 0
	 * 
	 * @param a X component of normal vector
	 * @param b Y component of normal vector
	 * @param c Z component of normal vector
	 * @param d Distance from origin (signed)
	 */
	void setCoefficients( float a, float b, float c, float d );
	
	/**
	 * @brief Calculate distance from point to plane
	 * 
	 * Computes the shortest signed distance from a point to the plane.
	 * Positive values are on the side of the plane pointed to by the normal.
	 * 
	 * @param p Point to measure distance from
	 * @return float Signed distance to the plane
	 * @retval 0 if point lies on the plane
	 * @retval > 0 if point is on the positive normal side
	 * @retval < 0 if point is on the negative normal side
	 */
	float distance(const lsVector3 &p );

	/// Debug output to console
	void debug();


	/// Normal vector of the plane (perpendicular direction)
	lsVector3 normal;
	
	/// A point on the plane
	lsVector3 point;
	
	/// Distance coefficient (plane equation: ax + by + cz + d = 0)
	float d;

};

#endif /* LSPLANE_H_ */
