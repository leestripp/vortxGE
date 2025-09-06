#ifndef vortxGE_lsPassManager_h
#define vortxGE_lsPassManager_h

class lsPass;
class lsOpenGL;

using namespace std;

class lsPassManager
{
public:
    lsPassManager( lsOpenGL* opengl );
    virtual ~lsPassManager();

    void Render();
	void Resize( int x, int y, int width, int height  );

    void Add_Pass( lsPass* pass );
    void Del_Pass( lsPass* pass );

    lsPass* getPass_list() const
    {
        return pass_list;
    }

    void setPass_list( lsPass* val )
    {
        pass_list = val;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);

private:
    lsPass      *pass_list;

    lsOpenGL *ogl;

};


#endif
