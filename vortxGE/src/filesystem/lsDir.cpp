// datablocks
#include "../datablocks/lsString.h"

#include "lsDir.h"

lsDir::lsDir()
{
}

lsDir::~lsDir()
{
}

string lsDir::getexepath()
{
  char proc[1024];
  string fullPath = "";
  string path = "";

  // build proc path
  pid_t pid = getpid();
  char buf[20] = {0};
  sprintf( buf, "%d", pid );
  string _link = "/proc/";
  _link.append( buf );
  _link.append( "/exe" );

  int ch = readlink( _link.c_str(), proc, 1024 );
  if( ch != -1 )
  {
    proc[ch] = 0;
    path = proc;
    string::size_type t = path.find_last_of("/");
    path = path.substr( 0, t );
  }
  fullPath = path + string("/");

  return fullPath;
}

