#ifndef vortxGE_lsControl_h
#define vortxGE_lsControl_h

#include <iostream>
#include <string>

class vortxGE;
class lsRender;
class lsControlManager;
class lsShaderManager;
class lsAssetManager;


// trpes
const int LSCT_NONE         = 0;
const int LSCT_COMPOSITE    = 1;
const int LSCT_MENU         = 2;
const int LSCT_VIEWER       = 3;
const int LSCT_LEVEL        = 4;
const int LSCT_LEVELLOADER  = 5;
const int LSCT_GAMEPLAY     = 6;
const int LSCT_GAME         = 7;
const int LSCT_SANDBOX      = 8;
const int LSCT_SANDBOXZONE  = 9;


using namespace std;


/**
 * @brief Base class
 *
 * */

class lsControl
{
public:
    lsControl( lsRender *render, lsControl* p );
    virtual ~lsControl();

    // Inti
    virtual int Init() = 0;

    virtual void Update( double delta );
    virtual void Render();
	virtual void Resize( int x, int y, int width, int height  );

    // Control hierarchy
    void Add_Control( lsControl* val );

    const string& getSave_string()
    {
        return save_string;
    }

    void setSave_string( const string& val )
    {
        save_string = val;
    }

    // Navi
    lsControl* getNext_control() const
    {
        return next_control;
    }

    void setNext_control( lsControl* val )
    {
        next_control = val;
    }

    lsControl* getPrev_control() const
    {
        return prev_control;
    }

    void setPrev_control( lsControl* val )
    {
        prev_control = val;
    }

    // Settings
    int getControl_type() const
    {
        return control_type;
    }

    void setControl_type( int val )
    {
        control_type = val;
    }

    const string& getName()
    {
        return name;
    }

    void setName( const string& val )
    {
        name = val;
    }

    bool getActive() const
    {
        return active;
    }

    void setActive( bool val )
    {
        active = val;
    }

    // Control hierarchy
    lsControlManager* getControlManager() const
    {
        return controlmanager;
    }

    lsControl* getParent() const
    {
        return parent;
    }

    lsRender *getRender() const;
    void setOgl(lsRender *value);

private:
    lsControl   *next_control;
    lsControl   *prev_control;
    // Control hierarchy
    lsControlManager *controlmanager;
    lsControl *parent;

    // render
    lsRender *m_render;

    int         control_type;
    string      name;
    bool        active;

    // Save
    string save_string;

};

#endif

