#pragma once

#include "lsLight.h"


class lsLight_Spot : public lsLight
{
public:
    lsLight_Spot(lsNode* parent);
    virtual ~lsLight_Spot();

    // Save data
    const string& toString() override;

private:

};
