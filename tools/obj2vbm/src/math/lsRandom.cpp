#include <cstdlib>
#include <ctime>
#include <iostream>

#include "lsRandom.h"


using namespace std;

lsRandom::lsRandom()
{
	srand((unsigned)time(0));

}

lsRandom::~lsRandom()
{
}

int lsRandom::Range( int max )
{
	return (rand()%max);
}
