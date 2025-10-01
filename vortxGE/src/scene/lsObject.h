#pragma once

// opengl
#include "../render/lsOpenGL.h"

// scene
#include "../scene/lsAnimNode.h"
#include "../scene/lsLightManager.h"

// Occlusion types;
const int LSOT_NONE         =	0;
const int LSOT_OCCLUDER		=	1;

class lsAsset;
class lsMesh;
class lsBulletObject;
class lsProperty;
class lsAnimationManager;


/** \brief Holds pointers to an Asset (.vbm mesh), Light Manager and Bullet Object.
 *
 * Remember to call "Init_Animation( double fps )" after creating an instance or no animation will be applied.
 */

class lsObject : public lsAnimNode
{
public:
	lsObject( lsNode* parent );
	virtual ~lsObject();
	
	virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
	virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
	virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
	
	// Occlusion
	void DrawSingle( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
	
	// Transforms : override lsNode
	void Set_Pos(float x, float y, float z);
	void Set_Rot(float x, float y, float z);
	void Set_Scale(float x, float y, float z);
	
	lsMesh* Get_Mesh();
	
	// Save data
	virtual const string& toString();

	void Set_Asset( lsAsset* asset );
	lsAsset* getAsset() const
	{
		return asset;
	}

	lsBulletObject* getBulletObject() const
	{
		return bulletobject;
	}

	void setBulletObject( lsBulletObject* val )
	{
		bulletobject = val;
	}

	lsLightManager* getLightManager() const
	{
		return lightmanager;
	}

	// rendering
	bool getcull_mesh() const
	{
		return cull_mesh;
	}

	void setCull_mesh( bool val )
	{
		cull_mesh = val;
	}

	bool getOccluded() const
	{
		return occluded;
	}

	void setOccluded( bool val )
	{
		occluded = val;
	}

	float getDistance_to_camera() const
	{
		return distance_to_camera;
	}

	void setDistance_to_camera( float val )
	{
		distance_to_camera = val;
	}

	int getOcclusion_type() const
	{
		return occlusion_type;
	}

	void setOcclusion_type( int val )
	{
		occlusion_type = val;
	}

	GLuint getOcclusionID() const
	{
		return occlusion_id;
	}

	void setOcclusionID( GLuint val )
	{
		occlusion_id = val;
	}


	// Draw --------------------------

	lsObject* getNext_drawobject() const
	{
		return next_drawobject;
	}

	void setNext_drawobject( lsObject* val )
	{
		next_drawobject = val;
	}

	lsObject* getPrev_drawobject() const
	{
		return prev_drawobject;
	}

	void setPrev_drawobject( lsObject* val )
	{
		prev_drawobject = val;
	}

	// Shadow flags
	bool getCasts_shadows() const { return casts_shadows; }
	void setCasts_shadows(bool val) { casts_shadows = val; }
	bool getReceives_shadows() const { return receives_shadows; }
	void setReceives_shadows(bool val) { receives_shadows = val; }
	
private:
	lsAsset             *asset;
	lsLightManager      *lightmanager;
	lsBulletObject      *bulletobject;

	// rendering draw list
	lsObject *next_drawobject;
	lsObject *prev_drawobject;

	// Culling
	bool cull_mesh;
	bool occluded;
	float distance_to_camera;
	int occlusion_type;
	GLuint occlusion_id;
	
	// Shadows
	bool casts_shadows;
	bool receives_shadows;
};
