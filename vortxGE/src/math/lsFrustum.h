/**
 * @file lsFrustum.h
 * @brief Frustum class for view volume culling
 * 
 * Defines a viewing frustum (pyramid with clipped top) used for
 * view frustum culling - determining which objects are visible
 * and should be rendered.
 * 
 * @author vortxGE
 * @date 2024
 */

#pragma once

#include "lsPlane.h"
#include "lsVector3.h"
#include "lsMatrix4x4.h"

/**
 * @enum FrustumSides
 * @brief Indices for the six planes of a frustum
 * 
 * The frustum consists of 6 planes that bound the view volume:
 * 4 side planes (TOP, BOTTOM, LEFT, RIGHT) and 2 end planes (NEARP, FARP).
 */
enum FrustumSides {
	TOP = 0,	///< Top clipping plane
	BOTTOM,		///< Bottom clipping plane
	LEFT,		///< Left clipping plane
	RIGHT,		///< Right clipping plane
	NEARP,		///< Near clipping plane
	FARP		///< Far clipping plane
};

/**
 * @enum FrustumLoc
 * @brief Location test results for frustum culling
 */
enum FrustumLoc {
	OUTSIDE,	///< Object is completely outside the frustum
	INTERSECT,	///< Object intersects the frustum boundary
	INSIDE		///< Object is completely inside the frustum
};

/*!
 * @class lsFrustum
 * @brief Defines the viewing volume for scene object culling
 * 
 * A frustum is a 3D shape that represents the portion of 3D space
 * that is visible from a particular viewpoint. It is commonly used
 * for view frustum culling - determining which objects in the scene
 * are potentially visible and therefore need to be rendered.
 * 
 * The frustum is defined by 6 planes:
 * - Near plane: closest visible distance
 * - Far plane: farthest visible distance
 * - Top, Bottom, Left, Right: define the vertical and horizontal bounds
 * 
 * @note The frustum can be set up for either perspective or orthographic
 * projection modes.
 * 
 * @example
 * @code
 * lsFrustum frustum;
 * frustum.Set_Perspective(60.0f, 16.0f/9.0f, 0.1f, 1000.0f);
 * 
 * lsVector3 cameraPos(0, 0, 10);
 * lsVector3 lookAt(0, 0, 0);
 * lsVector3 up(0, 1, 0);
 * frustum.Build_CullPlanes(cameraPos, lookAt, up);
 * 
 * // Test if object is visible
 * if (frustum.sphereInFrustum(objectPos, objectRadius) != OUTSIDE) {
 *     // Render object
 * }
 * @endcode
 */
class lsFrustum
{
public:
	/// Default constructor
	lsFrustum();
	
	/// Destructor
	~lsFrustum();
	
	/**
	 * @brief Set up perspective projection frustum
	 * 
	 * Creates a perspective frustum with the given field of view angle.
	 * Objects farther than fplane or closer than nplane are not visible.
	 * 
	 * @param view_angle Vertical field of view in degrees
	 * @param width_ratio Aspect ratio (width/height)
	 * @param nplane Distance to near clipping plane (must be > 0)
	 * @param fplane Distance to far clipping plane (must be > nplane)
	 */
	void Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane );
	
	/**
	 * @brief Set the view ratio (aspect ratio)
	 * 
	 * Updates the aspect ratio of the frustum without changing
	 * the field of view.
	 * 
	 * @param vr New aspect ratio (width/height)
	 */
	void Set_ViewRatio( float vr );;
	
	/**
	 * @brief Set up orthographic projection frustum
	 * 
	 * Creates an orthographic (parallel) frustum where objects
	 * don't get smaller with distance.
	 * 
	 * @param xMin Minimum X coordinate (left)
	 * @param xMax Maximum X coordinate (right)
	 * @param yMin Minimum Y coordinate (bottom)
	 * @param yMax Maximum Y coordinate (top)
	 * @param zMin Minimum Z coordinate (near)
	 * @param zMax Maximum Z coordinate (far)
	 */
	void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );

	/**
	 * @brief Build cull planes from a clip matrix (Gribb/Hartmann)
	 *
	 * Extracts the 6 frustum planes directly from proj*view — the
	 * exact volume the rasterizer clips against. Use this for
	 * "throwaway" cull cameras (e.g. CSM cascade fits) where no
	 * scene-graph camera exists and Build_CullPlanes' perspective
	 * bookkeeping (eye/look/up + near/far heights) is unavailable.
	 * Planes are normalized, normals face INWARD (distance>0 = inside,
	 * matching the SphereInFrustum/PointInFrustum tests).
	 */
	void Set_CullPlanesFromMatrix( const lsMatrix4x4& clip );
	
	/**
	 * @brief Build cull planes from camera parameters
	 * 
	 * Constructs the 6 frustum planes based on camera position,
	 * look direction, and up vector. This should be called each frame
	 * when the camera moves.
	 * 
	 * @param p Camera position
	 * @param l Point the camera is looking at
	 * @param u Camera up vector
	 */
	void Build_CullPlanes( const lsVector3& p, const lsVector3& l, const lsVector3& u );
	
	// Check objects
	
	/**
	 * @brief Calculate distance from a point to the frustum
	 * 
	 * @param V1 Point to measure from
	 * @return float Distance to nearest frustum plane
	 */
	float distance( const lsVector3& V1 );
	
	/**
	 * @brief Check if a point is inside the frustum
	 * 
	 * Tests whether a single point is within the viewing frustum.
	 * 
	 * @param p Point to test
	 * @return int Location result
	 * @retval INSIDE if point is inside all 6 planes
	 * @retval OUTSIDE if point is outside any plane
	 * @retval INTERSECT never returned for points
	 */
	int pointInFrustum( const lsVector3& p );
	
	/**
	 * @brief Check if a sphere intersects the frustum
	 * 
	 * Tests whether a sphere (point + radius) is within or intersecting
	 * the viewing frustum. Used for efficient culling of spherical objects.
	 * 
	 * @param p Center of the sphere
	 * @param raio Radius of the sphere
	 * @return int Location result
	 * @retval INSIDE if entire sphere is inside
	 * @retval INTERSECT if sphere intersects frustum boundary
	 * @retval OUTSIDE if sphere is completely outside
	 */
	int sphereInFrustum( const lsVector3& p, float raio );
	
	/**
	 * @brief Get the projection matrix
	 * 
	 * Returns the OpenGL-style projection matrix used by this frustum.
	 * 
	 * @return lsMatrix4x4& Reference to the projection matrix
	 */
	lsMatrix4x4& getProjectionMatrix()
	{
		return ProjectionMatrix;
	}

	/// Distance to the near clipping plane (L3.1 spot frustum sync)
	float getNearplane() const { return nearplane; }
	/// Distance to the far clipping plane
	float getFarplane() const { return farplane; }

	
private:
	/// The 6 planes defining the frustum boundaries
	lsPlane		pl[6];

	/// Distance to near clipping plane
	float nearplane, farplane;
	
	/// Height and width of near plane
	float near_height, near_width;
	
	/// Height and width of far plane
	float far_height, far_width;
	
	/// Aspect ratio and vertical field of view angle
	float ratio, angle, tang;
	
	/// Camera position in world space
	lsVector3 camerapos;
	
	/// OpenGL projection matrix
	lsMatrix4x4 ProjectionMatrix;
	
	/// Near plane corners (top-left, top-right, bottom-left, bottom-right)
	lsVector3 ntl, ntr, nbl, nbr;
	
	/// Far plane corners
	lsVector3 ftl, ftr, fbl, fbr;
};
