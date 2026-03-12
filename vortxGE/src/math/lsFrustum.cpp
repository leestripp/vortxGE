/**
 * @file lsFrustum.cpp
 * @brief View frustum implementation for frustum culling
 * 
 * This file implements the view frustum used for frustum culling -
 * the process of determining which objects are inside the camera's
 * view and should be rendered. Objects outside the frustum are
 * culled to improve rendering performance.
 * 
 * The frustum is defined by 6 planes:
 * - TOP, BOTTOM, LEFT, RIGHT (sides)
 * - NEARP, FARP (near and far clipping planes)
 * 
 * @see lsFrustum.h
 * @see lsPlane.h
 * @see lsCamera.h
 */

#include <cmath>

#include "lsMath.h"
#include "lsFrustum.h"


//=========================================================================
// Constructors
//=========================================================================

/**
 * @brief Default constructor
 * 
 * Creates a default perspective frustum with:
 * - 45° field of view
 * - 16:10 aspect ratio (1920/1200)
 * - Near plane: 1.0
 * - Far plane: 100.0
 */
lsFrustum::lsFrustum()
{
	// Default perspective setup
	Set_Perspective( 45, 1920.0 / 1200.0, 1.0, 100.0 );
}

/**
 * @brief Destructor
 */
lsFrustum::~lsFrustum()
{
}

//=========================================================================
// Projection Setup
//=========================================================================

/**
 * @brief Set perspective projection parameters
 * 
 * Configures the frustum for perspective projection.
 * This determines the shape of the viewing volume.
 * 
 * @param view_angle Vertical field of view (degrees)
 * @param width_ratio Aspect ratio (width/height)
 * @param nplane Distance to near clipping plane
 * @param fplane Distance to far clipping plane
 * 
 * @note Call when camera settings change or viewport is resized
 */
void lsFrustum::Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane )
{
	// Create projection matrix
	ProjectionMatrix.PerspectiveProjection( view_angle, width_ratio, nplane, fplane );

	// Store parameters
	nearplane = nplane;
	farplane = fplane;
	ratio = width_ratio;
	angle = view_angle;
	
	// Tangent of half the FOV (used for corner calculations)
	tang = (float)tan( ANG2RAD * angle * 0.5 );

	// Calculate frustum dimensions at near and far planes
	// These are used for building culling planes
	near_height = 2 * tang * nearplane;
	near_width = near_height * width_ratio;
	far_height = 2 * tang * farplane;
	far_width = far_height * width_ratio;
}

/**
 * @brief Update aspect ratio only
 * 
 * Convenience method to change viewport aspect ratio
 * without recomputing everything.
 * 
 * @param vr New aspect ratio (width/height)
 */
void lsFrustum::Set_ViewRatio(float vr)
{
	Set_Perspective( angle, vr, nearplane, farplane );
}

/**
 * @brief Set orthographic projection
 * 
 * Configures the frustum for orthographic (parallel) projection.
 * In orthographic view, there's no perspective foreshortening.
 * 
 * @param xMin Minimum X coordinate
 * @param xMax Maximum X coordinate
 * @param yMin Minimum Y coordinate
 * @param yMax Maximum Y coordinate
 * @param zMin Minimum Z coordinate (near)
 * @param zMax Maximum Z coordinate (far)
 */
void lsFrustum::Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
	// Create orthographic projection matrix
	ProjectionMatrix.OrthographicProjection( xMin, xMax, yMin, yMax, zMin, zMax );

	// Set frustum dimensions
	near_height = 2 * yMax;
	near_width = 2 * xMax;
	far_height = 2 * yMax;
	far_width = 2 * xMax;
}

//=========================================================================
// Frustum Plane Construction
//=========================================================================

/**
 * @brief Build culling planes from camera parameters
 * 
 * This is the core algorithm for frustum culling. It computes
 * the 6 planes that define the view frustum based on:
 * - Camera position (p)
 * - Look-at target (l)
 * - Up vector (u)
 * 
 * The algorithm:
 * 1. Compute camera's local coordinate system (X, Y, Z)
 * 2. Calculate corners of near and far planes
 * 3. Build each frustum plane from 3 points
 * 
 * @param p Camera position in world space
 * @param l Point camera is looking at
 * @param u Up direction vector
 * 
 * @note Planes are stored in counter-clockwise order
 */
void lsFrustum::Build_CullPlanes( const lsVector3& p, const lsVector3& l, const lsVector3& u )
{
	lsVector3 nc, fc, X, Y, Z;
    float nh, nw, fh, fw;

	// Store camera position
	camerapos = p;

	// ========================================
	// Step 1: Build camera coordinate system
	// ========================================
	
	// Compute Z axis (forward, pointing opposite to look direction)
	// In OpenGL, -Z is into the screen
	Z = p - l;
	Z.normalize();

	// X axis = up × Z (right vector)
	X = u * Z;
	X.normalize();
    
	// Y axis = Z × X (true up, orthogonalized)
	Y = Z * X;

    // ========================================
    // Step 2: Calculate half dimensions
    // ========================================
    nh = near_height * 0.5f;
    nw = near_width * 0.5f;
    fh = far_height * 0.5f;
    fw = far_width * 0.5f;

	// ========================================
	// Step 3: Compute center points of near/far planes
	// ========================================
	
	// Near plane center = camera position minus distance along Z
	nc = p - Z * nearplane;
	
	// Far plane center
	fc = p - Z * farplane;

	// ========================================
	// Step 4: Compute 8 corners of frustum
	// ========================================
	
	// Near plane corners (Top-Left, Top-Right, Bottom-Left, Bottom-Right)
	ntl = nc + Y * nh - X * nw;  // near top-left
	ntr = nc + Y * nh + X * nw;  // near top-right
	nbl = nc - Y * nh - X * nw;  // near bottom-left
	nbr = nc - Y * nh + X * nw;  // near bottom-right

	// Far plane corners
	ftl = fc + Y * fh - X * fw;  // far top-left
	ftr = fc + Y * fh + X * fw;  // far top-right
	fbl = fc - Y * fh - X * fw;  // far bottom-left
	fbr = fc - Y * fh + X * fw;  // far bottom-right

	// ========================================
	// Step 5: Build 6 culling planes
	// ========================================
	// Each plane is defined by 3 points in counter-clockwise order
	// (when viewed from outside the frustum)
	
	// Top plane
	pl[ TOP ].set3Points( ntr, ntl, ftl );
	
	// Bottom plane
	pl[ BOTTOM ].set3Points( nbl, nbr, fbr );
	
	// Left plane
	pl[ LEFT ].set3Points( ntl, nbl, fbl );
	
	// Right plane
	pl[ RIGHT ].set3Points( nbr, ntr, fbr );
	
	// Near plane
	pl[ NEARP ].set3Points( ntl, ntr, nbr );
	
	// Far plane
	pl[ FARP ].set3Points( ftr, ftl, fbl );
}

//=========================================================================
// Distance Calculations
//=========================================================================

/**
 * @brief Calculate distance from camera to a point
 * 
 * @param V1 Point to measure distance to
 * @return Distance in world units
 */
float lsFrustum::distance( const lsVector3& V1 )
{
	float dist;
	lsVector3 V3;

	V3 = camerapos - V1;
	dist = sqrt( V3.x*V3.x + V3.y*V3.y + V3.z*V3.z );

	return dist;
}

//=========================================================================
// Frustum Culling Tests
//=========================================================================

/**
 * @brief Test if a point is inside the frustum
 * 
 * Checks if the point is on the positive side of all
 * 6 frustum planes.
 * 
 * @param p Point to test
 * @return INSIDE if point is inside, OUTSIDE if outside
 */
int lsFrustum::pointInFrustum( const lsVector3& p )
{
	int result = INSIDE;
	
	// Test against all 6 planes
	for(int i=0; i < 6; i++) {
		// If point is behind any plane, it's outside
		if (pl[i].distance(p) < 0)
			return OUTSIDE;
	}
	
	return result;
}

/**
 * @brief Test if a sphere intersects the frustum
 * 
 * A sphere is defined by center point and radius.
 * 
 * Culling algorithm:
 * 1. For each plane, compute signed distance from sphere center
 * 2. If distance < -radius, sphere is completely outside (behind plane)
 * 3. Otherwise, sphere either intersects or is completely inside
 * 
 * @param p Sphere center
 * @param raio Sphere radius
 * @return INSIDE if at least partially visible, OUTSIDE if fully culled
 */
int lsFrustum::sphereInFrustum( const lsVector3& p, float raio )
{
	int result = INSIDE;
	float distance;

	// Test sphere against each plane
	for(int i=0; i < 6; i++)
	{
		// Get signed distance from plane to sphere center
		distance = pl[i].distance(p);
		
		// If completely behind this plane (distance < -radius)
		if( distance < -raio )
			return OUTSIDE;  // Sphere is culled

		/*
		 * Note: We don't check for intersection explicitly.
		 * If we reach here, sphere is either intersecting or inside.
		 * For simple culling, we just need to know if it's OUTSIDE.
		 * Being INSIDE or INTERSECT both mean we should render.
		 */
	}
	
	return result;
}
