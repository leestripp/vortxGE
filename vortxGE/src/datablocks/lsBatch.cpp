#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lsBatch.h"

lsBatch::lsBatch( lsRender *render )
{
    m_render = render;
	
    next_batch = NULL;
    prev_batch = NULL;

    batch_type = LSBT_NONE;
    name = "Not set";
    shader = NULL;
    vertex_count = 0;
	
}

lsBatch::~lsBatch()
{
#if defined(VORTX_DEBUG_BATCH)
    cout << "lsBatch::~lsBatch : Delete " << name << endl;
#endif

    // Batches are not assets, so must be deleted
    if( next_batch ) delete next_batch;
}

lsRender *lsBatch::getRender() const
{
    return m_render;
}

void lsBatch::setOgl(lsRender *value)
{
    m_render = value;
}

int lsBatch::getVertex_count() const
{
    return vertex_count;
}

void lsBatch::setVertex_count(int value)
{
    vertex_count = value;
}

