#include <cmath>

#include "lsMath.h"
#include "lsFrustum.h"


lsFrustum::lsFrustum()
{
	Set_Perspective( 45, 1920.0 / 1200.0, 1.0, 100.0 );

}

lsFrustum::~lsFrustum()
{
}

//! Perspective Projection
/*!
 * Only needs to be called if a cameras setting change or a viewport has been sized.
 *
 * */
void lsFrustum::Set_Perspective( float view_angle, float width_ratio, float nplane, float fplane )
{
	// create Projection matrix
	ProjectionMatrix.PerspectiveProjection( view_angle, width_ratio, nplane, fplane );

	// Store
	nearplane = nplane;
	farplane = fplane;
	ratio = width_ratio;
	angle = view_angle;
	tang = (float)tan( ANG2RAD * angle * 0.5 );

	// set geom
	near_height = 2 * tang * nearplane;
	near_width = near_height * width_ratio;
	far_height = 2 * tang* farplane;
	far_width = far_height * width_ratio;
}

void lsFrustum::Set_ViewRatio(float vr)
{
	Set_Perspective( angle, vr, nearplane, farplane );
}


void lsFrustum::Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax )
{
	// create Projection matrix
	ProjectionMatrix.OrthographicProjection( xMin, xMax, yMin, yMax, zMin, zMax );

	// set geom
	near_height = 2 * yMax;
	near_width = 2 * xMax;
	far_height = 2 * yMax;
	far_width = 2 * xMax;
}

void lsFrustum::Build_CullPlanes( const lsVector3& p, const lsVector3& l, const lsVector3& u )
{
	lsVector3 nc, fc, X, Y, Z;
    float nh, nw, fh, fw;

	// Store
	camerapos = p;

	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	Z = p - l;
	Z.normalize();

	// X axis of camera
	X = u * Z;
	X.normalize();
    
	// the real "up" vector is the cross product of Z and X
	Y = Z * X;

    // half heights and widths
    nh = near_height * 0.5f;
    nw = near_width * 0.5f;
    fh = far_height * 0.5f;
    fw = far_width * 0.5f;

	// compute the centers of the near and far planes
	nc = p - Z * nearplane;
	fc = p - Z * farplane;

	// compute the 4 corners of the frustum on the near plane
	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	// compute the 4 corners of the frustum on the far plane
	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;

	// compute the six planes
	// the function set3Points assumes that the points
	// are given in counter clockwise order
	pl[ TOP ].set3Points( ntr,ntl,ftl );
	pl[ BOTTOM ].set3Points (nbl,nbr,fbr );
	pl[ LEFT ].set3Points( ntl,nbl,fbl );
	pl[ RIGHT ].set3Points( nbr,ntr,fbr );
	pl[ NEARP ].set3Points( ntl,ntr,nbr );
	pl[ FARP ].set3Points( ftr,ftl,fbl );
}

float lsFrustum::distance( const lsVector3& V1 )
{
	float dist;
	lsVector3 V3;

	V3 = camerapos - V1;
	dist = sqrt( V3.x*V3.x + V3.y*V3.y + V3.z*V3.z );

	return dist;
}


int lsFrustum::pointInFrustum( const lsVector3& p )
{
	int result = INSIDE;
	for(int i=0; i < 6; i++) {

		if (pl[i].distance(p) < 0)
			return OUTSIDE;
	}
	return(result);
}


int lsFrustum::sphereInFrustum( const lsVector3& p, float raio )
{
	int result = INSIDE;
	float distance;

	for(int i=0; i < 6; i++)
	{
		distance = pl[i].distance(p);
		if( distance < -raio )
			return OUTSIDE;

		/*
		 *		if( distance < raio )
		 *			return INTERSECT;
		 */
	}
	return( result );
}


/* OLD code
void lsFrustum::drawGuide()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	glBegin(GL_QUADS);

	// Left
	glVertex3f( ftl.x, ftl.y,ftl.z );		// far top left
	glVertex3f( ntl.x, ntl.y, ntl.z );		// near top left
	glVertex3f( nbl.x, nbl.y, nbl.z );
	glVertex3f( fbl.x, fbl.y, fbl.z );

	// Right
	glVertex3f( ntr.x, ntr.y,ntr.z );		// far top left
	glVertex3f( ftr.x, ftr.y, ftr.z );		// near top left
	glVertex3f( fbr.x, fbr.y, fbr.z );
	glVertex3f( nbr.x, nbr.y, nbr.z );

	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
*/



