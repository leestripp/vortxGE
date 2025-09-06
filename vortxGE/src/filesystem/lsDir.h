#ifndef LSDIR_H_
#define LSDIR_H_

#include <limits.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

class lsDir
{
public:
  lsDir();
  virtual ~lsDir();

  string getexepath();

};

#endif
