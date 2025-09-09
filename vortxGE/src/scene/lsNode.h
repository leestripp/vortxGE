#pragma once

#include <string>
#include <thread>
#include <vector>

// math
#include "../math/lsMatrix4x4.h"
#include "../math/lsVector3.h"

// timers
#include "../timers/lsTimer.h"

// Node types
const int LSNT_NODE             =	0;
const int LSNT_OBJECT           =	1;
const int LSNT_TERRAIN          =	2;
const int LSNT_CAMERA           =	4;
const int LSNT_SCENE            =	5;
const int LSNT_MOTION           =	6;
const int LSNT_VEHICLE          =	7;
const int LSNT_FIRSTPERSONRIG   =	8;
const int LSNT_ZONE         	=  	9;
const int LSNT_DEBUG        	=   10;
const int LSNT_ACTOR        	=   11;
const int LSNT_LIGHT            =	12;
const int LSNT_EMPTY            =	13;
const int LSNT_OBJECT_LOD       =	14;
const int LSNT_OBJECT_EMITTER	=	15;
const int LSNT_PARTICLE_EMITTER	=	16;
const int LSNT_PARTICLE         =	17;
const int LSNT_ANIMNODE         =	18;
// Game items
const int LSNT_TRIGGER          =	100;
const int LSNT_PICKUP           =	101;

// protos
class lsScene;
class lsCamera;
class lsLight;
class lsPass;
class lsOpenGL;
class lsEventManager;

using namespace std;

/**
 Base class for all in scene objects
 @param class
 @returns none
 */
class lsNode
{
public:
    lsNode( lsNode* parent );
    virtual ~lsNode();
	
    // Override functions
    virtual void Move( lsScene* scn, lsOpenGL* opengl, double delta );
    virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
    virtual void InitDrawlist();
    // Threads
    void MoveThread( lsScene* scn, lsOpenGL* opengl, double delta );
    void OptimizeThread( lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl );
	
    // Scene
    virtual lsScene *Get_Scene();
    virtual lsEventManager *Get_EventManager();

    // Life
    void checkLife( lsScene *scn );
    void Set_Life( double val );
	
    // Node tree
    void Add_Node( lsNode *node );
    void Find_Node( lsScene* scn, const string& name );
    void Del_AllChildren();
    void Del_AllNodes();
	
    // Transforms
    virtual void Set_Pos(float x, float y, float z);
    virtual void Set_Rot(float x, float y, float z);
    virtual void Set_Scale(float x, float y, float z);
	
    // Save data
    void Save();
    virtual const string& toString() = 0;
    string save_str;
	
    // Navi --------------------------

    lsNode* getNext_node() const
    {
        return next_node;
    }

    void setNext_node( lsNode* val )
    {
        next_node = val;
    }

    lsNode* getPrev_node() const
    {
        return prev_node;
    }

    void setPrev_node( lsNode* val )
    {
        prev_node = val;
    }

    lsNode* getParent() const
    {
        return parent;
    }

    void setParent( lsNode* val )
    {
        parent = val;
    }

    lsNode* getNode_list() const
    {
        return node_list;
    }

    void setNode_list( lsNode* val )
    {
        node_list = val;
    }

    // Data --------------------------

    const string& getName()
    {
        return name;
    }

    void setName( const string& val )
    {
        name = val;
    }

    int getNode_type() const
    {
        return node_type;
    }

    void setNode_type( int val )
    {
        node_type = val;
    }

    // Public Data *****************************************************************
    lsMatrix4x4 ModelMatrix;
    lsVector3 position;
    lsVector3 rotation;
    lsVector3 scale;

private:
    lsNode  *next_node;
    lsNode  *prev_node;
    lsNode  *parent;
    lsNode  *node_list;

    // must be set.
    string name;
    int node_type;

    // Life
    lsTimer timer_life;
    double life;

};

