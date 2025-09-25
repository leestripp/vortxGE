#pragma once

#include "lsLight.h"

class lsCamera;

class lsLight_Direct : public lsLight
{
public:
    lsLight_Direct(lsNode* parent);
    virtual ~lsLight_Direct();
    
	// from lsNode
	virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
	
    // Save data
    const string& toString() override;
    
    // Camera following
    void setFollowCamera(lsCamera* camera) { m_followCamera = camera; }
    lsCamera* getFollowCamera() const { return m_followCamera; }
	
	// public data
    lsVector3 m_lightDirection;		// Calculate direction based on camera as we move with it.
    
private:
    lsCamera* m_followCamera;		// Camera to follow (if any)
};
