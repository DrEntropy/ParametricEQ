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

   const std::map<FilterType,juce::String>  filterToString
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


}
