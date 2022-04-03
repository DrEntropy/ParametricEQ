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

namespace FilterInfo {

    enum FilterType {
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
          {   {FirstOrderLowPass, "First Order Low Pass"},
              {FirstOrderHighPass,"First Order High Pass"},
              {FirstOrderAllPass,"First Order All Pass"},
              {LowPass, "Low Pass"},
              {HighPass,"High Pass"},
              {BandPass,"Band Pass"},
              {Notch, "Notch"},
              {AllPass, "All Pass"},
              {LowShelf, "Low Shelf"},
              {HighShelf, "High Shelf"},
              {PeakFilter, "Peak Filter"}
          };


}
