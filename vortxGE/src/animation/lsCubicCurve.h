/**
 * @file lsCubicCurve.h
 * @brief Cubic spline interpolation for smooth animations
 * 
 * lsCubicCurve implements Catmull-Rom spline interpolation
 * for smooth keyframe animation. It provides:
 * - Keyframe management (add, delete, find)
 * - Smooth interpolation between keyframes
 * - Efficient lookup for animation playback
 * 
 * @section interpolation Interpolation
 * 
 * Uses Catmull-Rom spline for smooth curves that
 * pass through all keyframes. The spline is defined
 * by the coefficient matrix M.
 * 
 * @section usage Usage
 * @code
 * lsCubicCurve* curve = new lsCubicCurve();
 * 
 * // Add keyframes
 * curve->Add_Key(0.0, 0.0);
 * curve->Add_Key(30.0, 10.0);
 * curve->Add_Key(60.0, 0.0);
 * 
 * // Get interpolated value
 * double value = curve->Get_Value(15.0);  // Midway
 * @endcode
 * 
 * @see lsKeyFrame for keyframe data
 * @see lsAnimationManager for animation system
 */

#ifndef LSCUBICCURVE_H_
#define LSCUBICCURVE_H_

#include <iostream>

//=========================================================================
// Catmull-Rom Spline Coefficients
//=========================================================================

/** @brief Spline matrix coefficient M11 */
#define M11	0.0
/** @brief Spline matrix coefficient M12 */
#define M12	1.0
/** @brief Spline matrix coefficient M13 */
#define M13	0.0
/** @brief Spline matrix coefficient M14 */
#define M14	0.0
/** @brief Spline matrix coefficient M21 */
#define M21	-0.5
/** @brief Spline matrix coefficient M22 */
#define M22	0.0
/** @brief Spline matrix coefficient M23 */
#define M23	0.5
/** @brief Spline matrix coefficient M24 */
#define M24	0.0
/** @brief Spline matrix coefficient M31 */
#define M31	1.0
/** @brief Spline matrix coefficient M32 */
#define M32	-2.5
/** @brief Spline matrix coefficient M33 */
#define M33	2.0
/** @brief Spline matrix coefficient M34 */
#define M34	-0.5
/** @brief Spline matrix coefficient M41 */
#define M41	-0.5
/** @brief Spline matrix coefficient M42 */
#define M42	1.5
/** @brief Spline matrix coefficient M43 */
#define M43	-1.5
/** @brief Spline matrix coefficient M44 */
#define M44	0.5

// Forward declaration
class lsKeyFrame;

/**
 * @class lsCubicCurve
 * @brief Cubic spline curve for keyframe interpolation
 * 
 * Implements Catmull-Rom spline interpolation
 * for smooth animation curves.
 * 
 * @section algorithm Algorithm
 * 
 * The spline uses the formula:
 * P(t) = t³M + t²M + tM + M
 * 
 * Where M is the 4x4 coefficient matrix.
 */
class lsCubicCurve
{
public:
    /**
     * @brief Default constructor
     */
    lsCubicCurve();
    
    /**
     * @brief Destructor
     */
    virtual ~lsCubicCurve();

    //=========================================================================
	// Keyframe Management
	//=========================================================================
	
    /**
     * @brief Add keyframe to curve
     * @param frame Frame number
     * @param value Keyframe value
     */
    void Add_Key( double frame, double value );
    
    /**
     * @brief Delete keyframe
     * @param key Keyframe to remove
     */
    void Del_Key( lsKeyFrame* key );
    
    /**
     * @brief Insert key before existing key
     * @param key Existing keyframe
     * @param newkey New keyframe to insert
     */
    void InsertKey_Before( lsKeyFrame* key, lsKeyFrame* newkey );
    
    /**
     * @brief Add keyframe to list
     * @param key Keyframe to add
     */
    void Add_ToList( lsKeyFrame* key );
    
    /**
     * @brief Clear all keyframes
     */
    void Clear_AllKeys();
    
    /**
     * @brief Find keyframe at frame
     * @param frame Frame number
     * @return Pointer to keyframe, or nullptr
     */
    lsKeyFrame* Find_Key( double frame );
    
    /**
     * @brief Check if curve is empty
     * @return true if no keyframes
     */
    bool Empty();

    //=========================================================================
	// Interpolation
	//=========================================================================
	
    /**
     * @brief Get interpolated value
     * 
     * Calculates the interpolated value at the
     * given frame using Catmull-Rom spline.
     * 
     * @param frame Frame number (can be between keyframes)
     * @return Interpolated value
     */
    double Get_Value( double frame );
    
    /**
     * @brief Catmull-Rom spline calculation
     * 
     * Internal method for spline interpolation.
     * 
     * @param x Interpolation parameter
     * @param v0 First control point
     * @param v1 Second control point
     * @param v2 Third control point
     * @param v3 Fourth control point
     * @return Interpolated value
     */
    double catmullRomSpline( double x, double v0, double v1, double v2, double v3 );

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Keyframe list head */
    lsKeyFrame* keyframe_list;

    /** @brief Number of keyframes */
    int keyframe_count;
    
    /** @brief Current position in list */
    double currpos;

    //=========================================================================
	// Current Keyframes (for interpolation)
	//=========================================================================
	
    /** @brief First control point */
    lsKeyFrame* key0;
    
    /** @brief Second control point */
    lsKeyFrame* key1;
    
    /** @brief Third control point */
    lsKeyFrame* key2;
    
    /** @brief Fourth control point */
    lsKeyFrame* key3;

};

#endif /* LSCUBICCURVE_H_ */
