#include <iostream>

// core
#include "../vortx_debug.h"

#include "lsPass.h"
#include "lsPassManager.h"


lsPassManager::lsPassManager( lsRender* render )
{
    pass_list = NULL;
    m_render = render;
}

lsPassManager::~lsPassManager()
{
    while( pass_list )
    {
        Del_Pass(pass_list);
    }
}

void lsPassManager::Render()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPassManager::Render" << endl;
#endif

	lsPass* pass;

	pass = pass_list;
	while( pass )
	{
        // Update
        pass->Update();
        // Render
        pass->Render();

		pass = pass->getNext_pass();
	}
}

void lsPassManager::Resize( int x, int y, int width, int height  )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPassManager::Resize" << endl;
#endif

	lsPass* pass;

	pass = pass_list;
	while( pass )
	{
		pass->Resize( x, y, width, height );
		pass = pass->getNext_pass();
	}
}

void lsPassManager::Add_Pass( lsPass *pass )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPassManager::Add_Pass : " << pass->getName() << endl;
#endif

    pass->setNext_pass( pass_list );
	if( pass_list ) pass_list->setPrev_pass( pass );
	pass_list = pass;
}

void lsPassManager::Del_Pass( lsPass *pass )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPassManager::Del_Pass : " << pass->getName() << endl;
#endif
    if(! pass  ) return;

	if( pass->getPrev_pass() == NULL )
	{
		// first in list
		pass_list = pass->getNext_pass();
		if( pass_list ) pass_list->setPrev_pass(0);

	} else
	{
		// Close the gap
		pass->getPrev_pass()->setNext_pass( pass->getNext_pass() );
		if( pass->getNext_pass() )
		{
			pass->getNext_pass()->setPrev_pass( pass->getPrev_pass() );
		}
	}
	delete pass;
}

lsRender *lsPassManager::getOgl() const
{
    return m_render;
}

void lsPassManager::setOgl(lsRender *value)
{
    m_render = value;
}

