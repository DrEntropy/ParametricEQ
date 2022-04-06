/*
  ==============================================================================

    FilterParametersBase.h
    Created: 3 Apr 2022 10:54:47am
    Author:  Ronald Legere
    
  ==============================================================================
*/

#pragma once



struct FilterParametersBase
{
    float frequency {1000.f};
    bool bypassed {false};
    float quality {1.0f};
    double sampleRate {20000.0};
    
   
};

inline bool operator==(const FilterParametersBase& lhs, const FilterParametersBase& rhs)
{
    return (lhs.frequency == rhs.frequency && lhs.bypassed == rhs.bypassed && lhs.quality == rhs.quality && lhs.sampleRate == rhs.sampleRate );
}

inline bool operator!=(const FilterParametersBase& lhs, const FilterParametersBase& rhs)
{
    return !(lhs == rhs);
}
