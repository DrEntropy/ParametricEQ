/*
  ==============================================================================

    FilterParameters.h
    Created: 3 Apr 2022 10:55:14am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include "FilterParametersBase.h"
#include "FilterInfo.h"

class FilterParameters : public FilterParametersBase
{
public:
    FilterInfo::FilterType filterType;
    float gain;
};
