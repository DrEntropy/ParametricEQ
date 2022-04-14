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


juce::String createGainParamString(int filterNum);

juce::String createQParamString(int filterNum);

juce::String createFreqParamString(int filterNum);

juce::String createBypassParamString(int filterNum);

juce::String createTypeParamString(int filterNum);

juce::String createSlopeParamString(int filterNum);
