#ifndef vortxGE_lsAnimNode_h
#define vortxGE_lsAnimNode_h

// prototypes
class lsAnimationManager;
class lsProperty;

#include "lsNode.h"

/** \brief Includes lsAnimationManager
 *
 * Subclass this node to create your own animation class.\n
 *
 */
class lsAnimNode : public lsNode
{
public:
    lsAnimNode( lsNode* p );
    virtual ~lsAnimNode();

	// Override functions
	virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
	virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
	virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    // Scene
    virtual lsScene *Get_Scene();
    virtual lsEventManager *Get_EventManager();

	// animation
    void Init_Animation( double fps );
    void Play_Animation( bool loop );
    void Stop_Animation();

	// Properties
	void Add_Property( lsProperty *p );

    // Save data
    virtual const string& toString();

    lsAnimationManager* getAnimationManager() const
    {
        return animationmanager;
    }

    lsProperty* getProperty_list() const
    {
        return property_list;
    }

private:
    lsAnimationManager  *animationmanager;

    // Properties
    lsProperty          *property_list;

};


#endif
