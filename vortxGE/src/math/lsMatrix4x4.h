/**
 * @file lsMatrix4x4.h
 * @brief 4x4 Transformation matrix for 3D graphics
 * 
 * lsMatrix4x4 is the primary transformation matrix used throughout
 * the engine for:
 * - Model transformations (position, rotation, scale)
 * - View matrices (camera transformation)
 * - Projection matrices (perspective/orthographic)
 * - Combined Model-View-Projection (MVP) matrices
 * 
 * The matrix uses column-major order (OpenGL convention):
 * @code
 * | m[0]  m[4]  m[8]  m[12] |
 * | m[1]  m[5]  m[9]  m[13] |
 * | m[2]  m[6]  m[10] m[14] |
 * | m[3]  m[7]  m[11] m[15] |
 * @endcode
 * 
 * Where:
 * - m[0-3]: First column (X axis)
 * - m[4-7]: Second column (Y axis)
 * - m[8-11]: Third column (Z axis)
 * - m[12-15]: Fourth column (translation)
 * 
 * @section transform_order Transformation Order
 * 
 * In column-major OpenGL, transformations are applied in reverse order.
 * For standard model matrix: Scale -> Rotate -> Translate
 * 
 * @see lsMatrix3x3 for rotation/scale only matrices
 * @see lsQuaternion for rotation representation
 */

#pragma once

#include <iostream>
#include <string>
#include <memory>

// Forward declarations
class lsVector3;
class lsQuaternion;

using namespace std;

 /**
 * @class lsMatrix4x4
 * @brief 4x4 transformation matrix for 3D graphics
 * 
 * Supports:
 * - Basic operations (multiply, transpose)
 * - Transformation building (rotation, translation, scale)
 * - Projection matrices (perspective, orthographic)
 * - Camera view matrices (LookAt)
 * - Conversion to/from quaternions and Euler angles
 */
class lsMatrix4x4
{
public:
    /**
     * @brief Default constructor
     * 
     * Creates an identity matrix.
     */
	lsMatrix4x4();
	
	//=========================================================================
	// Operators
	//=========================================================================
	
    /**
     * @brief Assignment from float array
     * @param mat4 16-element float array (column-major)
     * @return Reference to this matrix
     */
	lsMatrix4x4& operator=( const float mat4[16] );
	
    /**
     * @brief Matrix multiplication (this * B)
     * @param B Matrix to multiply
     * @return New matrix result
     */
	lsMatrix4x4 operator*( const lsMatrix4x4& B );
	
    /**
     * @brief Multiply by float array
     * @param mat4 16-element float array
     * @return New matrix result
     */
	lsMatrix4x4 operator*( const float mat4[16] );
	
    /**
     * @brief Transform a vector
     * 
     * Applies this matrix transformation to a 3D point.
     * Performs homogeneous division (w component).
     * 
     * @param v Input vector (treated as point)
     * @return Transformed vector
     */
	lsVector3 operator*( const lsVector3 &v);
	
	//=========================================================================
	// Transformation Methods
	//=========================================================================
	
    /**
     * @brief Rotate vector by this matrix
     * 
     * Applies rotation only (no translation).
     * Uses 3x3 upper-left submatrix.
     * 
     * @param v Input vector
     * @return Rotated vector
     */
	lsVector3 Rotate(const lsVector3 &v);
	
    /**
     * @brief Set from quaternion with translation
     * 
     * Creates a transformation matrix from rotation
     * (quaternion) and position.
     * 
     * @param q Quaternion representing rotation
     * @param x X translation
     * @param y Y translation
     * @param z Z translation
     */
	void Set(const lsQuaternion &q, float x=0, float y=0, float z=0);
	
    /**
     * @brief Load identity matrix
     * 
     * Resets matrix to identity:
     * @code
     * | 1 0 0 0 |
     * | 0 1 0 0 |
     * | 0 0 1 0 |
     * | 0 0 0 1 |
     * @endcode
     */
	void LoadIdentity();
	
    /**
     * @brief Apply rotation to matrix
     * 
     * Multiplies current matrix by rotation matrix.
     * 
     * @param angle Rotation angle in degrees
     * @param x X component of rotation axis
     * @param y Y component of rotation axis
     * @param z Z component of rotation axis
     */
	void RotateMatrix( float angle, float x, float y, float z );
	
    /**
     * @brief Apply translation to matrix
     * 
     * Multiplies current matrix by translation matrix.
     * 
     * @param x X translation
     * @param y Y translation
     * @param z Z translation
     */
	void TranslateMatrix( float x, float y, float z );
	
    /**
     * @brief Apply scale to matrix
     * 
     * Multiplies current matrix by scale matrix.
     * 
     * @param x X scale factor
     * @param y Y scale factor
     * @param z Z scale factor
     */
	void ScaleMatrix( float x, float y, float z );
	
	//=========================================================================
	// Projection Methods
	//=========================================================================
	
    /**
     * @brief Create perspective projection matrix
     * 
     * Standard perspective projection with field of view.
     * 
     * @param fovy Vertical field of view in degrees
     * @param aspect_ratio Width/height ratio
     * @param near_plane Near clipping plane (must be > 0)
     * @param far_plane Far clipping plane (must be > near)
     */
	void PerspectiveProjection( float fovy, float aspect_ratio, float near_plane, float far_plane );
	
    /**
     * @brief Create orthographic projection matrix
     * 
     * Parallel projection without perspective.
     * 
     * @param xMin Minimum X coordinate
     * @param xMax Maximum X coordinate
     * @param yMin Minimum Y coordinate
     * @param yMax Maximum Y coordinate
     * @param zMin Minimum Z coordinate (near)
     * @param zMax Maximum Z coordinate (far)
     */
	void OrthographicProjection( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
	
    /**
     * @brief Calculate cotangent
     * 
     * Utility for projection calculations.
     * cot(θ) = 1/tan(θ)
     * 
     * @param angle Angle in radians
     * @return Cotangent of angle
     */
	float Cotangent(float angle);
	
    /**
     * @brief Create view matrix (camera transformation)
     * 
     * Creates a matrix that transforms world coordinates
     * to camera (view) coordinates.
     * 
     * @param eyePosition Camera position in world space
     * @param lookAt Point the camera is looking at
     * @param upVector Up direction (typically 0,1,0)
     */
	void LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector );
	
    /**
     * @brief Transpose matrix
     * @param m Source matrix to transpose
     * @return Reference to this matrix (modified)
     */
	lsMatrix4x4& transpose( const lsMatrix4x4& m );
	
    /**
     * @brief Extract rotation component only
     * 
     * Returns the upper-left 3x3 submatrix containing
     * only rotation (no translation or scale).
     * 
     * @return New matrix with rotation only
     */
	lsMatrix4x4 extractRotation();
	
    /**
     * @brief Extract local transform from parent
     * 
     * Given a parent matrix, extracts the local transformation
     * that was applied to produce it.
     * 
     * @param parentMatrix The parent/world matrix
     * @return Local transformation matrix
     */
	lsMatrix4x4 getLocalTransformFromParent(const lsMatrix4x4& parentMatrix) const;
	
	//=========================================================================
	// Static Utility Methods
	//=========================================================================
	
    /**
     * @brief Convert matrix to Euler angles (ZYX order)
     * 
     * Extracts rotation as Euler angles in ZYX order
     * (yaw, pitch, roll).
     * 
     * @param m Input matrix
     * @return Euler angles in degrees (x=pitch, y=yaw, z=roll)
     */
	static lsVector3 ToEulerZYX(const lsMatrix4x4& m);
	
	//=========================================================================
	// Comparison and Testing
	//=========================================================================
	
    /**
     * @brief Compare matrices for equality
     * @param mat Matrix to compare
     * @return true if matrices are equal (within margin of error)
     */
	bool isEqual( const lsMatrix4x4& mat );
	
    /**
     * @brief Check if matrix is identity
     * @return true if matrix is identity matrix
     */
	bool isIdentity();
	
    /**
     * @brief Debug output
     * @param str Label for output
     */
    void debug( const string& str );

	//=========================================================================
    // Public Data Members
    //=========================================================================
	
    /**
     * @brief Matrix data (column-major, 16 elements)
     * 
     * Access via:
     * - matrix[0-15] for raw access
     * - or use transformation methods
     */
	float matrix[16];
	
    /**
     * @brief Margin of error for comparisons
     * 
     * Used by isEqual() for floating-point comparison.
     * Default value is typically 0.0001f
     */
	float moe;
};
