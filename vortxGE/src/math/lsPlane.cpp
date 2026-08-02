#include "lsVector3.h"
#include "lsPlane.h"
#include "lsMath.h"


lsPlane::lsPlane()
{

}

lsPlane::lsPlane( lsVector3 &v1,  lsVector3 &v2, lsVector3 &v3)
{
	set3Points(v1,v2,v3);
}

lsPlane::~lsPlane()
{
}



void lsPlane::set3Points( lsVector3 &v1,  lsVector3 &v2,  lsVector3 &v3 )
{
	lsMath m;
	lsVector3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = aux2 * aux1;

	normal.normalize();
	point = v2;
	d = -m.Dot( normal, point );
}

void lsPlane::setNormalAndPoint( const lsVector3 &normal, const lsVector3 &point )
{
	lsMath m;

	this->normal = normal;
	this->normal.normalize();
	d = -m.Dot( normal, point );
}

void lsPlane::setCoefficients(float a, float b, float c, float d)
{
	// set the normal vector
	normal = lsVector3(a,b,c);
	float l = normal.length();
	normal.normalize();
	this->d = d/l;
}


float lsPlane::distance(const lsVector3 &p )
{
	lsMath m;
	return d + m.Dot( normal, p );
}

void lsPlane::debug()
{
    normal.debug( "Plane(" );
	cout << ") " << d << endl;

}

// drawGuide() removed: was legacy fixed-function immediate-mode GL,
// deprecated since OpenGL 3.0 (2010).  A modern implementation should
// route through lsBatch_Lines or the renderer abstraction.
