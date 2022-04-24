/*
  ==============================================================================

    CoefficientsMaker.h
    Created: 4 Apr 2022 1:15:23pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FilterInfo.h"
#include "FilterParameters.h"
#include "HighCutLowCutParameters.h"
#include "HelperFunctions.h"



struct CoefficientsMaker
{
   
    
    static auto makeCoefficients (FilterInfo::FilterType filterType,
                                  float freq, float quality, float gain, double sampleRate)
    {
      using namespace FilterInfo;
      using namespace juce::dsp::IIR;
        
      switch (filterType)
      {
          case FilterType::FirstOrderLowPass:
              return Coefficients<float>::makeFirstOrderLowPass(sampleRate, freq);
              
          case FilterType::FirstOrderHighPass:
              return Coefficients<float>::makeFirstOrderHighPass(sampleRate, freq);
              
          case FilterType::FirstOrderAllPass:
              return Coefficients<float>::makeFirstOrderAllPass(sampleRate, freq);
              
          case FilterType::LowPass:
              return Coefficients<float>::makeLowPass(sampleRate, freq, quality);
              
          case FilterType::HighPass:
              return Coefficients<float>::makeHighPass(sampleRate, freq, quality);
              
          case FilterType::BandPass:
              return Coefficients<float>::makeBandPass(sampleRate, freq, quality);
              
          case FilterType::Notch:
              return Coefficients<float>::makeNotch(sampleRate, freq, quality);
              
          case FilterType::AllPass:
              return Coefficients<float>::makeAllPass(sampleRate, freq, quality);
              
          case FilterType::LowShelf:
              return Coefficients<float>::makeLowShelf(sampleRate, freq, quality, gain);
              
          case FilterType::HighShelf:
              return Coefficients<float>::makeHighShelf(sampleRate, freq, quality, gain);
              
          case FilterType::PeakFilter:
              return Coefficients<float>::makePeakFilter(sampleRate, freq, quality, gain);
              
      }
    }
    
    static auto makeCoefficients (FilterParameters filterParams)
    {
        return makeCoefficients(filterParams.filterType, filterParams.frequency, filterParams.quality, filterParams.gain.getGain(), filterParams.sampleRate);
    }
    
    
    static auto makeCoefficients (HighCutLowCutParameters filterParams)
    {
        if (filterParams.isLowcut)
        {
            return HelperFunctions::designIIRHighpassHighOrderButterworthMethod(filterParams.frequency,
                                                                                               filterParams.sampleRate,
                                                                                               filterParams.order, filterParams.quality);
        }
        
        return HelperFunctions::designIIRLowpassHighOrderButterworthMethod(filterParams.frequency,
                                                                                          filterParams.sampleRate,
                                                                                          filterParams.order, filterParams.quality);
    
    }
    
};



// convenience types

using FilterCoeffPtr = decltype(CoefficientsMaker::makeCoefficients (FilterParameters()));
using CutCoeffArray = decltype(CoefficientsMaker::makeCoefficients (HighCutLowCutParameters()));
