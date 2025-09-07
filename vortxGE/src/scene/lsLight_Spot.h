#ifndef LSLIGHT_SPOT_H
#define LSLIGHT_SPOT_H

#include "lsLight.h"

/**
 A spot light scene object
 */
class lsLight_Spot : public lsLight
{
public:
    lsLight_Spot(lsNode* parent);
    virtual ~lsLight_Spot();

    // Save data
    const string& toString() override;

private:

};

#endif // LSLIGHT_SPOT_H
