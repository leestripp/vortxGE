#include "lsRender.h"

lsRender::lsRender() :
	m_stats(nullptr),
	m_shaderManager(nullptr)
{
}

lsRender::~lsRender()
{
	// Managers are owned and deleted by vortxGE, so no deletion here.
}
