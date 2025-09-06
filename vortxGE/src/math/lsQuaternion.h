#ifndef LSQUATERNION_H_
#define LSQUATERNION_H_

#define DELTA 1e-10     // error tolerance

//! Quaternion
/*!
 * Not used, so the code was never finished.
 *
 */
class lsQuaternion
{
public:
	lsQuaternion();
	lsQuaternion(float w, float x, float y, float z);
	lsQuaternion(float roll, float pitch, float yaw);
	virtual ~lsQuaternion();

	lsQuaternion& operator*=( const lsQuaternion& q );
	lsQuaternion operator*( const lsQuaternion& q2 );

	void Set(float w, float x, float y, float z);
	void Slerp(const lsQuaternion &to, float t);
	void Slerp(const lsQuaternion &from, const lsQuaternion &to, float t);
	void Lerp(const lsQuaternion &to, float t);

	// Date
	float w, x, y, z;

};

#endif /* LSQUATERNION_H_ */
