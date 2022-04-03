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

struct FilterParameters :  FilterParametersBase
{
    FilterInfo::FilterType filterType {FilterInfo::FilterType::LowPass};
    float gain {0.f};
};
