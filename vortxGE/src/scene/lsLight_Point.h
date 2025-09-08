#ifndef vortxGE_IOS_lsLight_Point_h
#define vortxGE_IOS_lsLight_Point_h

#include "lsLight.h"

/**
 A point light scene object
 */
class lsLight_Point : public lsLight
{
public:
    lsLight_Point( lsNode *parent );
    virtual ~lsLight_Point();

    // Save data
    const string& toString();

private:

};


#endif
