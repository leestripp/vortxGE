#pragma once

#include "lsLight.h"

class lsLight_Direct : public lsLight
{
public:
    lsLight_Direct(lsNode* parent);
    virtual ~lsLight_Direct();

    // Save data
    const string& toString() override;

private:

};
