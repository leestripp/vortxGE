#ifndef LSLIGHTPTR_H_
#define LSLIGHTPTR_H_

class lsLight;

//! Light pointer
/*!
 * Holds a pointer to a light.
 * lsNodes use this.
 *
 */
class lsLightPtr
{
public:
	lsLightPtr();
	virtual ~lsLightPtr();


	lsLightPtr *getNext_lightptr() const
	{
		return next_lightptr;
	}

	void setNext_lightptr(lsLightPtr *next_lightptr)
	{
		this->next_lightptr = next_lightptr;
	}

	lsLightPtr *getPrev_lightptr() const
	{
		return prev_lightptr;
	}

	void setPrev_lightptr(lsLightPtr *prev_lightptr)
	{
		this->prev_lightptr = prev_lightptr;
	}

	lsLight *getLight() const
	{
		return light;
	}

	void setLight(lsLight *light)
	{
		this->light = light;
	}

private:
	lsLightPtr* next_lightptr;
	lsLightPtr* prev_lightptr;

	lsLight* light;

};

#endif /* LSLIGHTPTR_H_ */
