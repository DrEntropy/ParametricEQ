/*
  ==============================================================================

    FilterInfo.h
    Created: 2 Apr 2022 10:02:19pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <map>
#include <JuceHeader.h>

namespace FilterInfo
{

   enum class FilterType
   {
        FirstOrderLowPass,
        FirstOrderHighPass,
        FirstOrderAllPass,
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        LowShelf,
        HighShelf,
        PeakFilter
    };

    enum class Slope
    {
        Slope_6,
        Slope_12,
        Slope_18,
        Slope_24,
        Slope_30,
        Slope_36,
        Slope_42,
        Slope_48
    };

    const std::map<FilterType,juce::String>  mapFilterTypeToString
    {
        {FilterType::FirstOrderLowPass, "First Order Low Pass"},
        {FilterType::FirstOrderHighPass,"First Order High Pass"},
        {FilterType::FirstOrderAllPass,"First Order All Pass"},
        {FilterType::LowPass, "Low Pass"},
        {FilterType::HighPass,"High Pass"},
        {FilterType::BandPass,"Band Pass"},
        {FilterType::Notch, "Notch"},
        {FilterType::AllPass, "All Pass"},
        {FilterType::LowShelf, "Low Shelf"},
        {FilterType::HighShelf, "High Shelf"},
        {FilterType::PeakFilter, "Peak Filter"}
       
    };

    const std::map<Slope,juce::String>  mapSlopeToString
    {
        {Slope::Slope_6,"6 dB/Oct"},
        {Slope::Slope_12,"12 dB/Oct"},
        {Slope::Slope_18,"18 dB/Oct"},
        {Slope::Slope_24,"24 dB/Oct"},
        {Slope::Slope_30,"30 dB/Oct"},
        {Slope::Slope_36,"36 dB/Oct"},
        {Slope::Slope_42,"42 dB/Oct"},
        {Slope::Slope_48,"48 dB/Oct"}
    };


}
