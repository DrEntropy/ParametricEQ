/*
  ==============================================================================

    ParameterHelpers.cpp
    Created: 3 Apr 2022 2:14:53pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "ParameterHelpers.h"

juce::String createParamString(juce::String label, int filterNum)
{
    return "Filter_" + juce::String(filterNum)+"_"+ label;
}


juce::String createGainParamString(int filterNum)
{
    return createParamString("gain",filterNum);
}

juce::String createQParamString(int filterNum)
{
    return createParamString("Q",filterNum);
}

juce::String createFreqParamString(int filterNum)
{
    return createParamString("freq",filterNum);
}

juce::String createBypassParamString(int filterNum)
{
    return createParamString("bypass",filterNum);
}

juce::String createTypeParamString(int filterNum)
{
    return createParamString("type",filterNum);
}


juce::String createSlopeParamString(int filterNum)
{
    return createParamString("slope",filterNum);
}
