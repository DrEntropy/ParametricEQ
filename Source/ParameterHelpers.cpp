/*
  ==============================================================================

    ParameterHelpers.cpp
    Created: 3 Apr 2022 2:14:53pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "ParameterHelpers.h"

juce::String ParamString(juce::String label, int filterNum)
{
    return "Filter_" + juce::String(filterNum)+"_"+ label;
}


juce::String createGainParamString(int filterNum)
{
    return ParamString("gain",filterNum);
}

juce::String createQParamString(int filterNum)
{
    return ParamString("Q",filterNum);
}

juce::String createFreqParamString(int filterNum)
{
    return ParamString("freq",filterNum);
}

juce::String createBypassParamString(int filterNum)
{
    return ParamString("bypass",filterNum);
}

juce::String createTypeParamString(int filterNum)
{
    return ParamString("type",filterNum);
}
