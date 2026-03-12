#include <iostream>
#include <cassert>

#include "mesh/lsMesh.h"


int main(int argc, char **argv)
{
    std::cout << "obj2vbm for vortxGE by Lee A. Stripp" << std::endl;
    std::cout << "====================================" << std::endl;

    if( argc > 1 )
    {
        lsMesh *mesh = new lsMesh;
        assert( mesh != NULL );

        if( mesh->Load_Mesh( argv[1] ) )
        {
            // export the data
            mesh->exportVBM( argv[1] );
        }

        // Cleanup
        delete mesh;

    } else
    {
        cout << "Usage :         .obj file" << endl;
        cout << "        obj2vbm filename" << endl;
    }

    return 0;
}

