/*
  ==============================================================================

    ParameterHelpers.h
    Created: 3 Apr 2022 2:14:53pm
    Author:  Ronald Legere
 
    Helper functions to create parameter names

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FilterInfo.h"


juce::String GainParamString(int filterNum);

juce::String QParamString(int filterNum);

juce::String FreqParamString(int filterNum);

juce::String BypassParamString(int filterNum);

juce::String TypeParamString(int filterNum);
