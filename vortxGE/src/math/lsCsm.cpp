#include "lsCsm.h"

#include <cmath>

std::vector<float> lsCsmSplitDepths( float nearZ, float farZ,
									 int cascades, float lambda )
{
	std::vector<float> out( cascades + 1 );
	for( int i = 0; i <= cascades; ++i )
	{
		const float si = (float)i / (float)cascades;
		const float logS = nearZ * powf( farZ / nearZ, si );
		const float uniS = nearZ + ( farZ - nearZ ) * si;
		out[i] = lambda * logS + ( 1.0f - lambda ) * uniS;
	}
	return out;
}

std::vector<lsCsmCascade> lsCsmCompute( const lsCsmConfig& cfg,
										const lsMatrix4x4& camView,
										float fovYDeg, float aspect,
										float nearZ, float farZ,
										const lsVector3& lightDir,
										bool yFlip )
{
	std::vector<lsCsmCascade> out;
	int N = cfg.cascades;
	if( N < 1 ) N = 1;
	if( N > 4 ) N = 4;
	if( nearZ <= 0.0f ) nearZ = 0.1f;
	if( cfg.maxDistance > 0.0f && farZ > cfg.maxDistance )
		farZ = cfg.maxDistance;
	if( farZ <= nearZ ) return out;

	// --- camera basis + world position from the view matrix ----------
	// rows of the view rotation are the camera axes in world space
	const float* m = camView.matrix;
	const lsVector3 right( m[0], m[4], m[8] );
	const lsVector3 up( m[1], m[5], m[9] );
	const lsVector3 back( m[2], m[6], m[10] );
	const lsVector3 fwd = -back;
	// camPos = -(R^T * t); R^T t = right*t.x + up*t.y + back*t.z
	const lsVector3 camPos = -( right * m[12] + up * m[13] + back * m[14] );

	// --- light frame (rotation only, stable for a fixed sun) ---------
	lsVector3 f = lightDir;
	f.normalize();
	const lsVector3 up0 = ( fabsf( f.y ) > 0.99f )
		? lsVector3( 1.0f, 0.0f, 0.0f ) : lsVector3( 0.0f, 1.0f, 0.0f );
	lsMatrix4x4 lightRot;
	lightRot.LookAt( lsVector3( 0.0f, 0.0f, 0.0f ), f, up0 );
	// rows of lightRot's rotation (for the inverse below)
	const float* lr = lightRot.matrix;
	const lsVector3 lrRight( lr[0], lr[4], lr[8] );
	const lsVector3 lrUp( lr[1], lr[5], lr[9] );
	const lsVector3 lrBack( lr[2], lr[6], lr[10] );

	const std::vector<float> splits =
		lsCsmSplitDepths( nearZ, farZ, N, cfg.lambda );

	const float tanHalf = tanf( fovYDeg * 0.5f * 3.14159265358979f / 180.0f );

	for( int i = 0; i < N; ++i )
	{
		const float d0 = splits[i], d1 = splits[i + 1];

		// 8 slice corners in world space
		lsVector3 corners[8];
		int k = 0;
		for( int s = 0; s < 2; ++s )
		{
			const float d = s ? d1 : d0;
			const float h = d * tanHalf;
			const float w = h * aspect;
			const lsVector3 centre = camPos + fwd * d;
			corners[k++] = centre + right * w + up * h;
			corners[k++] = centre - right * w + up * h;
			corners[k++] = centre + right * w - up * h;
			corners[k++] = centre - right * w - up * h;
		}

		// bounding sphere (rotation-invariant fit)
		lsVector3 C( 0.0f, 0.0f, 0.0f );
		for( int j = 0; j < 8; ++j ) C += corners[j];
		C = C * ( 1.0f / 8.0f );
		float r = 0.0f;
		for( int j = 0; j < 8; ++j )
		{
			const float d = ( corners[j] - C ).length();
			if( d > r ) r = d;
		}
		if( r <= 0.0f ) r = 1.0f;

		// L2.2: texel snapping anchored to the WORLD, not the slice —
		// transform the centre through the light ROTATION only, snap
		// to the cascade texel grid, and un-rotate back to world.
		const lsVector3 C_rot = lightRot * C;
		const float texel = ( 2.0f * r ) / (float)cfg.mapSize;
		lsVector3 C_snap = C_rot;
		C_snap.x = floorf( C_rot.x / texel + 0.5f ) * texel;
		C_snap.y = floorf( C_rot.y / texel + 0.5f ) * texel;
		// lightRot^T * C_snap = lrRight*x + lrUp*y + lrBack*z
		const lsVector3 Cw = lrRight * C_snap.x + lrUp * C_snap.y +
							 lrBack * C_snap.z;

		// eye one radius behind the slice (plus one more as caster
		// margin: ortho zMin reaches one radius behind the eye)
		const float dist = 2.0f * r;
		const lsVector3 eye = Cw - f * dist;

		lsCsmCascade cc;
		cc.view.LookAt( eye, Cw, up0 );
		// depth span: [-r behind eye, 3r in front] = 4r total
		cc.proj.OrthographicProjection( -r, r, -r, r, -r, 3.0f * r );

		// bias: NDC [-1,1] -> texture [0,1] (+ VK V-mirror convention)
		float bias[] = { 0.5f, 0.0f, 0.0f, 0.0f,
						   0.0f, 0.5f, 0.0f, 0.0f,
						   0.0f, 0.0f, 0.5f, 0.0f,
						   0.5f, 0.5f, 0.5f, 1.0f };
		if( yFlip ) bias[5] = -0.5f;
		lsMatrix4x4 biasMatrix;
		biasMatrix = bias;

		cc.shadowMatrix = biasMatrix * cc.proj * cc.view;
		cc.splitNear = d0;
		cc.splitFar = d1;
		out.push_back( cc );
	}
	return out;
}
