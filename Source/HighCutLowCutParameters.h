/*
  ==============================================================================

    HighCutLowCutParameters.h
    Created: 3 Apr 2022 10:55:54am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once



#include "FilterParametersBase.h"

struct HighCutLowCutParameters : FilterParametersBase
{
    // this will be 1-8, where the value represents a 6db/oct slope.  1 = 6db/oct slope
    // 8 = 48 db/oct
    int order {1};
    bool isLowcut {true};
};