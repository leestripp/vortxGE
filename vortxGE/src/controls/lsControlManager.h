#ifndef vortxGE_lsControlManager_h
#define vortxGE_lsControlManager_h

#include <iostream>

class lsControl;
class lsRender;

using namespace std;

/*!
 * \brief Manage other control objects including Composite and Viewer.
 *
 * When building your pipeline add you controls in the right order.\n
 * Add a Composite Control first so it becomes the last item rendered.\n
 * Controls are added into the top of the list.\n
 * \n
 * Parent class is vortxGE.\n
 * vortxGE calls the Update() and Render() functions.\n
 * Resize() sould only be called by the user if the GL context has changed e.g. A Window Resize..\n
 * \n
 */
class lsControlManager
{
public:
    lsControlManager( lsRender* render );
    virtual ~lsControlManager();

    void Update( double delta );
    void Render();
	void Resize( int x, int y, int width, int height  );

    void Add_Control( lsControl* control );
    void Del_Control( lsControl* control );

    lsControl* getControl_list() const
    {
        return control_list;
    }

    lsRender *getOgl() const;
    void setOgl(lsRender *value);

private:
    lsControl *control_list;

    lsRender *m_render;

};

#endif
