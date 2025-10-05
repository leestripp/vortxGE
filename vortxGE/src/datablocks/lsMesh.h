#pragma once

#include <iostream>
#include <string>

// math
#include "../math/lsBoundingSphere.h"

#include "lsData.h"

class lsBatch;
class lsObject;
class lsCamera;
class lsPass;
class lsRender;
class lsAssetManager;
class lsShaderManager;

using namespace std;

/*!
 * \brief The lsMesh class
 *
 * Manage a list of lsBatch objects for rendering.\n
 *  Subclassed from lsData.\n
 *
 */
class lsMesh : public lsData
{
public:
	lsMesh( lsRender *render );
	virtual ~lsMesh();

	void Draw( lsCamera *cam, lsPass *pass, lsObject *object );
	void Update_Animation();

	void LoadVBM( const string& filename, lsAssetManager *am, lsShaderManager *sm );
	void LoadGLTF( const string& filename, lsAssetManager *am, lsShaderManager *sm );
	void Add_Batch( lsBatch* batch );

	const lsBoundingSphere& getBSphere()
	{
		return bSphere;
	}

	lsBatch* getBatch_list() const
	{
		return batch_list;
	}

	lsRender *getRender() const;
	void setOgl(lsRender *value);

private:
	lsBatch* batch_list;

	// Overall around all batches : Needs implementing
	lsBoundingSphere bSphere;
	// render
	lsRender *m_render;

};
