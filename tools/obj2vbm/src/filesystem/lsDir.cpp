#include "../convert/lsString.h"

#include "lsDir.h"

lsDir::lsDir()
{
}

lsDir::~lsDir()
{
}

#if defined(__linux__)
string lsDir::getexepath()
{
	lsString convert;
	vector<string> list;
	string path;
	string exe_path;
    char slash;

	slash = '/';
	path = "/"; // linux root

	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	exe_path =  string( result, (count > 0) ? count : 0 );

	if( exe_path != "" )
	{
        convert.SplitString( exe_path, slash, list );

		for( int i=0; i<(int)list.size()-1; i++ )
		{
			path += list[i] + slash;
		}
	} else
	{
		path = "";
	}
	return path;
}
#endif /* __linux__ */


#if defined(__APPLE__)
string lsDir::getexepath()
{
	lsString convert;
	vector<string> list;
	string path;
	string exe_path;
	string slash;
	slash = '/';
	path = "/"; // linux root

	char result[1024];
	uint32_t size = sizeof(result);
	if( _NSGetExecutablePath(result, &size) == 0 )
		exe_path = result;
	else
		exe_path = "";

	if( exe_path != "" )
	{
		convert.SplitString( exe_path, slash, list, false );

		for( int i=0; i<(int)list.size()-1; i++ )
		{
			path += list[i] + slash;
		}
	} else
	{
		path = "";
	}
	return path;
}
#endif /* __APPLE__ */

