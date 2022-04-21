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
#include "Decibel.h"

struct FilterParameters :  FilterParametersBase
{
    FilterInfo::FilterType filterType {FilterInfo::FilterType::LowPass};
    Decibel<float> gain {0.f};
};


inline bool operator==(const FilterParameters& lhs, const FilterParameters& rhs)
{
    return (lhs.filterType == rhs.filterType && lhs.gain == rhs.gain &&
            static_cast<const FilterParametersBase&>(lhs) == static_cast<const FilterParametersBase&>(rhs) );
}


inline bool operator!=(const FilterParameters& lhs, const FilterParameters& rhs)
{
    return !(lhs == rhs);
}
