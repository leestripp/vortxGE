#ifndef LSCUBICCURVE_H_
#define LSCUBICCURVE_H_

#include <iostream>


/* Coefficients for Matrix M */
#define M11	 0.0
#define M12	 1.0
#define M13	 0.0
#define M14	 0.0
#define M21	-0.5
#define M22	 0.0
#define M23	 0.5
#define M24	 0.0
#define M31	 1.0
#define M32	-2.5
#define M33	 2.0
#define M34	-0.5
#define M41	-0.5
#define M42	 1.5
#define M43	-1.5
#define M44	 0.5

class lsKeyFrame;

using namespace std;

//! Cubic Curve
/*!
 * Cubic Curve Interpolation
 *
 */
class lsCubicCurve
{
public:
    lsCubicCurve();
    virtual ~lsCubicCurve();

    void Add_Key( double frame, double value );
    void Del_Key( lsKeyFrame* key );
    void InsertKey_Before( lsKeyFrame* key, lsKeyFrame* newkey );
    void Add_ToList( lsKeyFrame* key );
    void Clear_AllKeys();
    lsKeyFrame* Find_Key( double frame );
    bool Empty();

    double Get_Value( double frame );
    double catmullRomSpline( double x, double v0, double v1, double v2, double v3 );

private:
    lsKeyFrame* keyframe_list;

    int keyframe_count;
    double currpos;

    // current keys
    lsKeyFrame* key0;
    lsKeyFrame* key1;
    lsKeyFrame* key2;
    lsKeyFrame* key3;

};

#endif /* LSCUBICCURVE_H_ */
