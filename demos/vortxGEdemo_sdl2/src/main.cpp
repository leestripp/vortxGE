#include <iostream>
#include "sdlApp.h"

using namespace std;


int main()
{
	sdlApp app;

	// Create GL window and instance demo scene
	if( app.Init() )
	{
		// Game loop
		app.Run();
	} else
	{
		cout << "ERROR: sdlApp::Init Failed." << endl;
		return 1;
	}
    return 0;
}
