#ifndef vortxGE_lsShaderManager_h
#define vortxGE_lsShaderManager_h

#include <iostream>

class lsShader;
class lsOpenGL;

using namespace std;

/**
 Manage global shaders
 @param class
 @returns none
 */
class lsShaderManager
{
public:
    lsShaderManager( lsOpenGL *opengl );
    virtual ~lsShaderManager();
    // opengl
    lsOpenGL *ogl;

    void Init();
    void Add_Shader( lsShader* shader );
    void Del_Shader( lsShader* shader );

    lsShader* Find_Shader( const string& name );

    lsShader* getShaderList() const
    {
        return shader_list;
    }

private:
    lsShader *shader_list;

};


#endif
