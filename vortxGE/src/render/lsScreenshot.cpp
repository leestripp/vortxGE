#include "lsScreenshot.h"

#include <cstdio>
#include <cstring>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../datablocks/stb_image_write.h"

using namespace std;

bool lsScreenshot::saveRGB( const char* path, int w, int h, const uint8_t* rgb )
{
	if( ! path || ! rgb || w <= 0 || h <= 0 ) return false;

	const size_t len = strlen( path );
	const bool png = len > 4 && strcmp( path + len - 4, ".png" ) == 0;

	bool ok = false;
	if( png )
	{
		ok = stbi_write_png( path, w, h, 3, rgb, w * 3 ) != 0;
	}
	else
	{
		FILE* f = fopen( path, "wb" );
		if( f )
		{
			fprintf( f, "P6\n%d %d\n255\n", w, h );
			ok = fwrite( rgb, 1, (size_t)w * (size_t)h * 3, f )
				 == (size_t)w * (size_t)h * 3;
			fclose( f );
		}
	}
	if( ok )
		cout << "SCREENSHOT written: " << path << " (" << w << "x" << h
			 << ( png ? ", png" : ", ppm" ) << ")" << endl;
	else
		cerr << "SCREENSHOT failed:  " << path << endl;
	return ok;
}
