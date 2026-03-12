/**
 * @file lsMatrix4x4.cpp
 * @brief 4x4 Matrix implementation for 3D transformations
 * 
 * This file contains the implementation of lsMatrix4x4, which provides
 * matrix operations for 3D graphics including:
 * - Matrix multiplication
 * - Rotation, translation, and scaling
 * - Perspective and orthographic projections
 * - LookAt view matrix generation
 * - Euler angle extraction
 * - Transform decomposition
 * 
 * @note Matrices are stored in column-major order (OpenGL standard)
 * @note All angles are in degrees unless otherwise specified
 * 
 * @see lsMatrix4x4.h
 * @see lsVector3.h
 * @see lsQuaternion.h
 */

#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>
#include <algorithm> // For clamp

#include "lsMatrix4x4.h"
#include "lsVector3.h"
#include "lsQuaternion.h"
#include "lsMath.h"


//=========================================================================
// Constructors
//=========================================================================

/**
 * @brief Default constructor
 * 
 * Initializes matrix to identity and sets margin of error (moe)
 * for floating-point comparisons.
 */
lsMatrix4x4::lsMatrix4x4()
{
    // margin of error for floating-point comparisons
    moe = 0.00001;
    LoadIdentity();
}

/**
 * @brief Assignment operator from float array
 * 
 * @param mat4 Source 4x4 matrix as 16-element array (column-major)
 * @return Reference to this matrix
 */
lsMatrix4x4& lsMatrix4x4::operator=( const float mat4[16] )
{
	memcpy( matrix, mat4, sizeof(float) * 16 );
	return *this;
}

//=========================================================================
// Basic Operations
//=========================================================================

/**
 * @brief Load identity matrix
 * 
 * Sets this matrix to the identity matrix:
 * [1 0 0 0]
 * [0 1 0 0]
 * [0 0 1 0]
 * [0 0 0 1]
 * 
 * The identity matrix is the multiplicative identity for matrix multiplication.
 * Any matrix multiplied by identity returns itself.
 */
void lsMatrix4x4::LoadIdentity()
{
    // identity
    matrix[ 0]=1;
    matrix[ 1]=0;
    matrix[ 2]=0;
    matrix[ 3]=0;

    matrix[ 4]=0;
    matrix[ 5]=1;
    matrix[ 6]=0;
    matrix[ 7]=0;

    matrix[ 8]=0;
    matrix[ 9]=0;
    matrix[10]=1;
    matrix[11]=0;

    matrix[12]=0;
    matrix[13]=0;
    matrix[14]=0;
    matrix[15]=1;
}

/**
 * @brief Rotate a vector by this matrix (rotation only, no translation)
 * 
 * Applies only the rotation portion of the matrix to the vector.
 * This is equivalent to multiplying by the upper-left 3x3 submatrix.
 * 
 * Useful for rotating direction vectors or normals without
 * applying translation (which would move the point).
 * 
 * @param v Input vector
 * @return Rotated vector
 * 
 * @note Does not apply translation component
 */
lsVector3 lsMatrix4x4::Rotate( const lsVector3& v )
{
	lsVector3 ret;
	
    // Multiply 3x3 rotation submatrix by vector
    // matrix[0], [1], [2] = first column
    // matrix[4], [5], [6] = second column
    // matrix[8], [9], [10] = third column
    ret.x = matrix[0]*v.x + matrix[4]*v.y + matrix[8]*v.z;
    ret.y = matrix[1]*v.x + matrix[5]*v.y + matrix[9]*v.z;
    ret.z = matrix[2]*v.x + matrix[6]*v.y + matrix[10]*v.z;

	return ret;
}

//=========================================================================
// Matrix Multiplication
//=========================================================================

/**
 * @brief Matrix multiplication operator
 * 
 * Computes C = A * B where this matrix is A and B is the input.
 * Matrix multiplication is NOT commutative: A * B != B * A in general.
 * 
 * This implements standard 4x4 matrix multiplication:
 * C[row,col] = sum(A[row,k] * B[k,col]) for k = 0 to 3
 * 
 * @param B Matrix to multiply by (right side)
 * @return Resulting matrix C = this * B
 * 
 * @note Column-major storage: matrix[col * 4 + row]
 * @note For transform combination, typically: world = parent * local
 */
lsMatrix4x4 lsMatrix4x4::operator*( const lsMatrix4x4& B )
{
    lsMatrix4x4 m;

    // Column 0 of result
    m.matrix[0] = matrix[0]*B.matrix[0] + matrix[4]*B.matrix[1] + matrix[8]*B.matrix[2] + matrix[12]*B.matrix[3];
    m.matrix[1] = matrix[1]*B.matrix[0] + matrix[5]*B.matrix[1] + matrix[9]*B.matrix[2] + matrix[13]*B.matrix[3];
    m.matrix[2] = matrix[2]*B.matrix[0] + matrix[6]*B.matrix[1] + matrix[10]*B.matrix[2] + matrix[14]*B.matrix[3];
    m.matrix[3] = matrix[3]*B.matrix[0] + matrix[7]*B.matrix[1] + matrix[11]*B.matrix[2] + matrix[15]*B.matrix[3];

    // Column 1 of result
    m.matrix[4] = matrix[0]*B.matrix[4] + matrix[4]*B.matrix[5] + matrix[8]*B.matrix[6] + matrix[12]*B.matrix[7];
    m.matrix[5] = matrix[1]*B.matrix[4] + matrix[5]*B.matrix[5] + matrix[9]*B.matrix[6] + matrix[13]*B.matrix[7];
    m.matrix[6] = matrix[2]*B.matrix[4] + matrix[6]*B.matrix[5] + matrix[10]*B.matrix[6] + matrix[14]*B.matrix[7];
    m.matrix[7] = matrix[3]*B.matrix[4] + matrix[7]*B.matrix[5] + matrix[11]*B.matrix[6] + matrix[15]*B.matrix[7];

    // Column 2 of result
    m.matrix[8] = matrix[0]*B.matrix[8] + matrix[4]*B.matrix[9] + matrix[8]*B.matrix[10] + matrix[12]*B.matrix[11];
    m.matrix[9] = matrix[1]*B.matrix[8] + matrix[5]*B.matrix[9] + matrix[9]*B.matrix[10] + matrix[13]*B.matrix[11];
    m.matrix[10] = matrix[2]*B.matrix[8] + matrix[6]*B.matrix[9] + matrix[10]*B.matrix[10] + matrix[14]*B.matrix[11];
    m.matrix[11] = matrix[3]*B.matrix[8] + matrix[7]*B.matrix[9] + matrix[11]*B.matrix[10] + matrix[15]*B.matrix[11];

    // Column 3 of result (includes translation)
    m.matrix[12] = matrix[0]*B.matrix[12] + matrix[4]*B.matrix[13] + matrix[8]*B.matrix[14] + matrix[12]*B.matrix[15];
    m.matrix[13] = matrix[1]*B.matrix[12] + matrix[5]*B.matrix[13] + matrix[9]*B.matrix[14] + matrix[13]*B.matrix[15];
    m.matrix[14] = matrix[2]*B.matrix[12] + matrix[6]*B.matrix[13] + matrix[10]*B.matrix[14] + matrix[14]*B.matrix[15];
    m.matrix[15] = matrix[3]*B.matrix[12] + matrix[7]*B.matrix[13] + matrix[11]*B.matrix[14] + matrix[15]*B.matrix[15];

    return m;
}

/**
 * @brief Matrix multiplication with float array
 * 
 * @param mat4 4x4 matrix as 16-element array
 * @return Result matrix
 */
lsMatrix4x4 lsMatrix4x4::operator*( const float mat4[16] )
{
    lsMatrix4x4 B;
    B = mat4;
    return (*this)*B;
}

/**
 * @brief Transform a 3D point by this matrix
 * 
 * Applies the full transformation including rotation and translation.
 * This is the standard homogeneous coordinate transformation:
 * [x']   [m00 m01 m02 m03] [x]
 * [y'] = [m10 m11 m12 m13] [y]
 * [z']   [m20 m21 m22 m23] [z]
 * [1 ]   [m30 m31 m32 m33] [1]
 * 
 * For affine transforms (rotation + translation), m30=m31=m32=0, m33=1
 * so: x' = m00*x + m01*y + m02*z + m03
 * 
 * @param v Input 3D point
 * @return Transformed 3D point
 */
lsVector3 lsMatrix4x4::operator*( const lsVector3 &v)
{
    lsVector3 ret;

    // Apply 3x3 rotation + translation column
    ret.x = matrix[0]*v.x + matrix[4]*v.y + matrix[8]*v.z + matrix[12];
    ret.y = matrix[1]*v.x + matrix[5]*v.y + matrix[9]*v.z + matrix[13];
    ret.z = matrix[2]*v.x + matrix[6]*v.y + matrix[10]*v.z + matrix[14];
    
    return ret;
}

//=========================================================================
// Transform Building Methods
//=========================================================================

/**
 * @brief Apply rotation around arbitrary axis
 * 
 * Creates a rotation matrix using Rodrigues' rotation formula.
 * The rotation is applied by multiplying the current matrix
 * on the right: this = this * rotation
 * 
 * Rodrigues' formula:
 * R = I + sin(θ)*K + (1-cos(θ))*K²
 * where K is the cross-product matrix of the normalized axis
 * 
 * @param angle Rotation angle in degrees
 * @param x X component of rotation axis
 * @param y Y component of rotation axis
 * @param z Z component of rotation axis
 * 
 * @note Axis is normalized before use
 * @note Positive angle follows right-hand rule
 */
void lsMatrix4x4::RotateMatrix( float angle, float x, float y, float z )
{
    lsMatrix4x4 m;
    m.LoadIdentity();

    // Convert to radians
    float rad = angle * ANG2RAD;
	
    // Normalise rotation axis to unit vector
    float length = sqrt(x*x + y*y + z*z);
    x /= length;
    y /= length;
    z /= length;

    // Compute rotation matrix components
    // Using Rodrigues' rotation formula
    float c = cos(rad);           // cos(θ)
    float s = sin(rad);           // sin(θ)
    float oneMinusC = 1.0f - c;   // 1 - cos(θ)

    // Build rotation matrix (column-major order for OpenGL)
    // First column
    m.matrix[0] = x*x*oneMinusC + c;
    m.matrix[4] = x*y*oneMinusC - z*s;
    m.matrix[8] = x*z*oneMinusC + y*s;
    m.matrix[12] = 0.0f;
    
    // Second column
    m.matrix[1] = y*x*oneMinusC + z*s;
    m.matrix[5] = y*y*oneMinusC + c;
    m.matrix[9] = y*z*oneMinusC - x*s;
    m.matrix[13] = 0.0f;

    // Third column
    m.matrix[2] = z*x*oneMinusC - y*s;
    m.matrix[6] = z*y*oneMinusC + x*s;
    m.matrix[10] = z*z*oneMinusC + c;
    m.matrix[14] = 0.0f;
    
    // Fourth column (w=1 for affine transform)
    m.matrix[3] = 0.0f;
    m.matrix[7] = 0.0f;
    m.matrix[11] = 0.0f;
    m.matrix[15] = 1.0f;

    // Apply rotation: this = this * rotation
    *this = (*this) * m;
}

/**
 * @brief Apply translation
 * 
 * Adds translation to the matrix by multiplying on the right:
 * this = this * translation_matrix
 * 
 * Translation matrix:
 * [1 0 0 tx]
 * [0 1 0 ty]
 * [0 0 1 tz]
 * [0 0 0 1 ]
 * 
 * @param x Translation along X axis
 * @param y Translation along Y axis
 * @param z Translation along Z axis
 */
void lsMatrix4x4::TranslateMatrix( float x, float y, float z )
{
    lsMatrix4x4 m;
    m.LoadIdentity();

    // Set translation components in fourth column
    m.matrix[12] = x;
    m.matrix[13] = y;
    m.matrix[14] = z;

    // Apply translation
    *this = (*this) * m;
}

/**
 * @brief Apply non-uniform scale
 * 
 * Applies scaling by multiplying on the right:
 * this = this * scale_matrix
 * 
 * Scale matrix:
 * [sx 0  0  0]
 * [0  sy 0  0]
 * [0  0  sz 0]
 * [0  0  0  1]
 * 
 * @param x Scale factor along X
 * @param y Scale factor along Y
 * @param z Scale factor along Z
 * 
 * @note Use 1.0f for no scaling on an axis
 */
void lsMatrix4x4::ScaleMatrix( float x, float y, float z )
{
    lsMatrix4x4 m;
    m.LoadIdentity();
    
    // Set diagonal scale components
    m.matrix[0] = x;
    m.matrix[5] = y;
    m.matrix[10] = z;

    // Apply scale
    *this = (*this) * m;
}

//=========================================================================
// Projection Matrices
//=========================================================================

/**
 * @brief Create perspective projection matrix
 * 
 * Creates a matrix that transforms coordinates into clip space
 * for perspective rendering. Objects further from the camera
 * appear smaller (parallel lines converge).
 * 
 * The projection maps the view frustum to normalized device
 * coordinates (NDC). The frustum is defined by:
 * - fovy: Vertical field of view (degrees)
 * - aspect_ratio: Width/height of viewport
 * - near_plane: Distance to near clipping plane
 * - far_plane: Distance to far clipping plane
 * 
 * Projection formula:
 * x_ndc = (f/aspect) * x / z
 * y_ndc = f * y / z
 * z_ndc = (far+near)/(near-far) + (2*far*near)/(near-far) * 1/z
 * 
 * @param fovy Vertical field of view in degrees
 * @param aspect_ratio Viewport aspect ratio (width/height)
 * @param near_plane Near clipping plane (must be > 0)
 * @param far_plane Far clipping plane (must be > near)
 */
void lsMatrix4x4::PerspectiveProjection( float fovy, float aspect_ratio, float near_plane, float far_plane )
{
    // Compute focal length from field of view
    // f = 1/tan(fovy/2)
    float f = Cotangent(fovy / 2.0f);

    matrix[0] = f / aspect_ratio;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    // Z mapping: maps z to [-1,1] in NDC
    // Negative row 2 makes depth increase into the screen (OpenGL convention)
    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (far_plane + near_plane) / (near_plane - far_plane);
    matrix[11] = -1.0f;

    // Z offset to maintain proper depth range
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
    matrix[15] = 0.0f;
}

/**
 * @brief Create orthographic projection matrix
 * 
 * Creates a matrix for parallel projection where objects
 * maintain the same size regardless of distance. No perspective
 * foreshortening occurs.
 * 
 * Maps axis-aligned bounding box to NDC coordinates [-1, 1].
 * 
 * @param xMin Minimum X world coordinate
 * @param xMax Maximum X world coordinate
 * @param yMin Minimum Y world coordinate
 * @param yMax Maximum Y world coordinate
 * @param zMin Minimum Z world coordinate (near)
 * @param zMax Maximum Z world coordinate (far)
 */
void lsMatrix4x4::OrthographicProjection( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
    // Scale to NDC range [-1, 1]
    matrix[0] = 2.0f / (xMax - xMin);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (yMax - yMin);
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -2.0f / (zMax - zMin);
    matrix[11] = 0.0f;

    // Translation to center the box at origin in NDC
    matrix[12] = -(xMax + xMin) / (xMax - xMin);
    matrix[13] = -(yMax + yMin) / (yMax - yMin);
    matrix[14] = -(zMax + zMin) / (zMax - zMin);
    matrix[15] = 1.0f;
}

/**
 * @brief Compute cotangent
 * 
 * @param angle Angle in degrees
 * @return Cotangent of angle (1/tan)
 */
float lsMatrix4x4::Cotangent(float angle)
{
    return 1.0f / tan(angle * ANG2RAD);
}

//=========================================================================
// View Matrix
//=========================================================================

/**
 * @brief Create LookAt view matrix
 * 
 * Constructs a view matrix that transforms world coordinates
 * to camera-local coordinates. The camera is positioned at
 * eyePosition and looking at lookAt point.
 * 
 * The view matrix is the inverse of the camera's world transform.
 * This implementation constructs it directly using:
 * 1. Compute camera's forward (z), right (x), and up (y) vectors
 * 2. Build rotation matrix from these basis vectors
 * 3. Apply translation to move eye to origin
 * 
 * @param eyePosition Camera position in world space
 * @param lookAt Point the camera is looking at
 * @param upVector Up direction (typically 0,1,0)
 */
void lsMatrix4x4::LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector )
{
    // Compute camera coordinate system (right-handed)
    
    // Forward: from lookAt to eye (into the screen is -Z in OpenGL)
    lsVector3 zaxis = (eyePosition - lookAt).normalize();
    
    // Right: cross product of up and forward
    lsVector3 xaxis = (upVector.Cross(zaxis)).normalize();
    
    // Up: cross product of forward and right (recompute for orthogonality)
    lsVector3 yaxis = zaxis.Cross(xaxis);

    // Build rotation matrix (columns are basis vectors)
    // Row 0 = xaxis (right)
    matrix[0] = xaxis.x;
    matrix[1] = yaxis.x;
    matrix[2] = zaxis.x;
    matrix[3] = 0.0f;

    // Row 1 = yaxis (up)
    matrix[4] = xaxis.y;
    matrix[5] = yaxis.y;
    matrix[6] = zaxis.y;
    matrix[7] = 0.0f;

    // Row 2 = zaxis (backward/forward)
    matrix[8] = xaxis.z;
    matrix[9] = yaxis.z;
    matrix[10] = zaxis.z;
    matrix[11] = 0.0f;

    // Apply translation (dot products give distance along each axis)
    matrix[12] = -xaxis.Dot(eyePosition);
    matrix[13] = -yaxis.Dot(eyePosition);
    matrix[14] = -zaxis.Dot(eyePosition);
    matrix[15] = 1.0f;
}

//=========================================================================
// Matrix Operations
//=========================================================================

/**
 * @brief Transpose matrix
 * 
 * Transpose swaps rows and columns: M'[i][j] = M[j][i]
 * For rotation matrices (orthogonal), inverse = transpose.
 * 
 * @param m Source matrix to transpose
 * @return Reference to this matrix
 */
lsMatrix4x4& lsMatrix4x4::transpose( const lsMatrix4x4& m )
{
    // Swap elements across main diagonal
    matrix[0] = m.matrix[0]; matrix[4] = m.matrix[1]; matrix[8] = m.matrix[2]; matrix[12] = m.matrix[3];
    matrix[1] = m.matrix[4]; matrix[5] = m.matrix[5]; matrix[9] = m.matrix[6]; matrix[13] = m.matrix[7];
    matrix[2] = m.matrix[8]; matrix[6] = m.matrix[9]; matrix[10] = m.matrix[10]; matrix[14] = m.matrix[11];
    matrix[3] = m.matrix[12]; matrix[7] = m.matrix[13]; matrix[11] = m.matrix[14]; matrix[15] = m.matrix[15];
    return *this;
}

/**
 * @brief Extract 3x3 rotation/scale submatrix
 * 
 * Extracts the upper-left 3x3 block containing rotation
 * and scale information. Translation components are set to zero.
 * 
 * Useful for extracting rotation matrix from a combined
 * transform that includes translation.
 * 
 * @return 4x4 matrix with 3x3 rotation and scale in upper-left
 */
lsMatrix4x4 lsMatrix4x4::extractRotation()
{
    lsMatrix4x4 rotationMatrix;
    rotationMatrix.LoadIdentity();

    // Copy the top-left 3x3 block (rotation and scale)
    rotationMatrix.matrix[0] = this->matrix[0];
    rotationMatrix.matrix[1] = this->matrix[1];
    rotationMatrix.matrix[2] = this->matrix[2];

    rotationMatrix.matrix[4] = this->matrix[4];
    rotationMatrix.matrix[5] = this->matrix[5];
    rotationMatrix.matrix[6] = this->matrix[6];

    rotationMatrix.matrix[8] = this->matrix[8];
    rotationMatrix.matrix[9] = this->matrix[9];
    rotationMatrix.matrix[10] = this->matrix[10];

    // Clear translation
    rotationMatrix.matrix[12] = 0.0f;
    rotationMatrix.matrix[13] = 0.0f;
    rotationMatrix.matrix[14] = 0.0f;
    
    return rotationMatrix;
}

//=========================================================================
// Comparison
//=========================================================================

/**
 * @brief Check if matrices are approximately equal
 * 
 * Uses margin of error (moe) for floating-point comparison.
 * Two values are considered equal if |a - b| <= moe
 * 
 * @param mat Matrix to compare against
 * @return true if all elements within tolerance
 */
bool lsMatrix4x4::isEqual(const lsMatrix4x4 &mat)
{
    for( int i=0; i<16; i++ )
    {
        // Check if outside margin of error
        if(( matrix[i] < (mat.matrix[i]-moe) )||( matrix[i] > (mat.matrix[i]+moe) )) return false;
    }
    return true;
}

/**
 * @brief Check if matrix is identity
 * 
 * Verifies matrix is approximately equal to identity.
 * Uses margin of error for floating-point comparison.
 * 
 * @return true if this is an identity matrix
 */
bool lsMatrix4x4::isIdentity()
{
    // Check diagonal elements are 1
    if(( matrix[ 0] < (1.0-moe) )||( matrix[ 0] > (1.0+moe) )) return false;
    if(( matrix[ 5] < (1.0-moe) )||( matrix[ 5] > (1.0+moe) )) return false;
    if(( matrix[10] < (1.0-moe) )||( matrix[10] > (1.0+moe))) return false;
    if(( matrix[15] < (1.0-moe) )||( matrix[15] > (1.0+moe))) return false;

    // Check off-diagonal elements are 0
    for( int i=0; i<16; i++ )
    {
        if( (i == 0) || (i == 5) || (i == 10) || (i == 15) ) continue;
        if(( matrix[i] < (0.0-moe) )||( matrix[i] > (0.0+moe) )) return false;
    }

    return true;
}

//=========================================================================
// Quaternion Conversion
//=========================================================================

/**
 * @brief Set matrix from quaternion and translation
 * 
 * Converts a quaternion to a rotation matrix and combines
 * with translation. This is the standard way to create
 * a transform matrix from rotation + position.
 * 
 * Quaternion to rotation matrix (for unit quaternion q):
 * R = [
 *   1-2(y²+z²)   2(xy+zw)   2(xz-yw)   0
 *   2(xy-zw)   1-2(x²+z²)   2(yz+xw)   0
 *   2(xz+yw)   2(yz-xw)   1-2(x²+y²)   0
 *   tx         ty         tz         1
 * ]
 * 
 * @param q Unit quaternion representing rotation
 * @param x Translation X
 * @param y Translation Y
 * @param z Translation Z
 */
void lsMatrix4x4::Set(const lsQuaternion &q, float x, float y, float z)
{
    // Set translation
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;

    // Set rotation from quaternion
    // Using the conversion formula for unit quaternions
    matrix[0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    matrix[1] = 2.0f * (q.x * q.y + q.z * q.w);
    matrix[2] = 2.0f * (q.x * q.z - q.y * q.w);
    matrix[3] = 0.0f;

    matrix[4] = 2.0f * (q.x * q.y - q.z * q.w);
    matrix[5] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    matrix[6] = 2.0f * (q.y * q.z + q.x * q.w);
    matrix[7] = 0.0f;

    matrix[8] = 2.0f * (q.x * q.z + q.y * q.w);
    matrix[9] = 2.0f * (q.y * q.z - q.x * q.w);
    matrix[10] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    matrix[11] = 0.0f;

    matrix[15] = 1.0f;
}

//=========================================================================
// Euler Angles
//=========================================================================

/**
 * @brief Extract Euler angles (roll, pitch, yaw) from matrix
 * 
 * Converts rotation matrix to Euler angles using ZYX rotation order
 * (common for camera/flight systems). Returns angles in degrees.
 * 
 * The extraction handles gimbal lock by detecting when pitch
 * approaches ±90° (cos(pitch) ≈ 0).
 * 
 * @param m Source rotation matrix
 * @return lsVector3(roll, pitch, yaw) in degrees
 * 
 * @note Gimbal lock occurs at pitch = ±90°, limiting yaw/roll separation
 */
lsVector3 lsMatrix4x4::ToEulerZYX(const lsMatrix4x4& m)
{
    float roll_rad, pitch_rad, yaw_rad;
    
    // Column-major matrix access: m.matrix[col * 4 + row]
    
    // Extract pitch (rotation.y) from m[2][0] = -sin(pitch)
    pitch_rad = asin(clamp(-m.matrix[2], -1.0f, 1.0f)); 
    
    // Check for gimbal lock (pitch near +/- 90 degrees)
    const float C_EPSILON = 1e-4f;
    if (fabs(cos(pitch_rad)) > C_EPSILON) 
    {
        // General case: No gimbal lock
        
        // yaw (rotation.z) from m[1][0] and m[0][0]
        yaw_rad = atan2(m.matrix[1], m.matrix[0]);
        
        // roll (rotation.x) from m[2][1] and m[2][2]
        roll_rad = atan2(m.matrix[6], m.matrix[10]);
    }
    else 
    {
        // Gimbal lock: pitch is +/- 90 degrees
        // Yaw and roll become coupled - set yaw to zero
        yaw_rad = 0.0f;

        // Calculate roll from remaining elements
        roll_rad = atan2(-m.matrix[4], m.matrix[5]);
    }
    
    // Convert radians to degrees
    return lsVector3(roll_rad * RAD2ANG, pitch_rad * RAD2ANG, yaw_rad * RAD2ANG);
}

//=========================================================================
// Transform Decomposition
//=========================================================================

/**
 * @brief Extract local transform from parent-child relationship
 * 
 * Given a world transform (this) and a parent world transform,
 * computes the local transform that, when applied in parent's
 * coordinate system, produces the world transform.
 * 
 * Mathematically: local = parent_inv * world
 * 
 * This is useful for:
 * - Maintaining object transforms when reparenting
 * - Editing local vs world transforms in editors
 * - Network synchronization of hierarchical transforms
 * 
 * @param parentMatrix Parent's world transform matrix
 * @return Local transform matrix (this = parent * local)
 */
lsMatrix4x4 lsMatrix4x4::getLocalTransformFromParent(const lsMatrix4x4& parentMatrix) const
{
    // To get local transform: local = parent_inverse * world
    
    // Create a copy of the current (world) matrix
    lsMatrix4x4 worldTransform = *this;
    
    // Create inverse of the parent matrix
    lsMatrix4x4 parentInverse;
    parentInverse.LoadIdentity();
    
    // For a standard transformation matrix (rotation + translation),
    // we can compute the inverse efficiently by:
    // 1. Transpose the rotation (inverse of rotation is its transpose)
    // 2. Negate and rotate the translation
    
    // Extract rotation part from parent (3x3 upper-left)
    lsMatrix4x4 parentRotationOnly;
    parentRotationOnly.LoadIdentity();
    parentRotationOnly.matrix[0] = parentMatrix.matrix[0];
    parentRotationOnly.matrix[1] = parentMatrix.matrix[1];
    parentRotationOnly.matrix[2] = parentMatrix.matrix[2];
    parentRotationOnly.matrix[4] = parentMatrix.matrix[4];
    parentRotationOnly.matrix[5] = parentMatrix.matrix[5];
    parentRotationOnly.matrix[6] = parentMatrix.matrix[6];
    parentRotationOnly.matrix[8] = parentMatrix.matrix[8];
    parentRotationOnly.matrix[9] = parentMatrix.matrix[9];
    parentRotationOnly.matrix[10] = parentMatrix.matrix[10];
    
    // Compute inverse of rotation matrix (transpose for orthonormal matrix)
    parentRotationOnly.transpose(parentRotationOnly);
    
    // Extract translation part from parent
    lsVector3 parentTranslation(parentMatrix.matrix[12], parentMatrix.matrix[13], parentMatrix.matrix[14]);
    
    // Apply inverse rotation to the translation, then negate
    lsVector3 invertedTranslation = parentRotationOnly.Rotate(parentTranslation);
    invertedTranslation.x = -invertedTranslation.x;
    invertedTranslation.y = -invertedTranslation.y;
    invertedTranslation.z = -invertedTranslation.z;
    
    // Set up inverse matrix with rotated and negated translation
    parentInverse.matrix[0] = parentRotationOnly.matrix[0];
    parentInverse.matrix[1] = parentRotationOnly.matrix[1];
    parentInverse.matrix[2] = parentRotationOnly.matrix[2];
    parentInverse.matrix[4] = parentRotationOnly.matrix[4];
    parentInverse.matrix[5] = parentRotationOnly.matrix[5];
    parentInverse.matrix[6] = parentRotationOnly.matrix[6];
    parentInverse.matrix[8] = parentRotationOnly.matrix[8];
    parentInverse.matrix[9] = parentRotationOnly.matrix[9];
    parentInverse.matrix[10] = parentRotationOnly.matrix[10];
    parentInverse.matrix[12] = invertedTranslation.x;
    parentInverse.matrix[13] = invertedTranslation.y;
    parentInverse.matrix[14] = invertedTranslation.z;
    
    // Compute local transform: parent_inverse * world_transform
    return parentInverse * worldTransform;
}

//=========================================================================
// Debug Output
//=========================================================================

/**
 * @brief Print matrix to console for debugging
 * 
 * Outputs matrix in human-readable format to std::cout.
 * Useful for debugging transform issues.
 * 
 * @param str Optional label printed before matrix
 */
void lsMatrix4x4::debug( const string& str )
{
    stringstream msg;
	
    msg << "Matrix : " << str << endl;
	
	int k;
	for( int i=0; i<4; i++ )
	{
		for( int j=0; j<16; j+=4 )
		{
			k = j+i;
            msg << "[ " << matrix[k] << " ]   ";
			
		}
        msg << endl;
	}
    cout << msg.str();
}
