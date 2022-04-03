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


juce::String GainParamString(int filterNum)
{
    return ParamString("gain",filterNum);
}

juce::String QParamString(int filterNum)
{
    return ParamString("Q",filterNum);
}

juce::String FreqParamString(int filterNum)
{
    return ParamString("freq",filterNum);
}

juce::String BypassParamString(int filterNum)
{
    return ParamString("bypass",filterNum);
}

juce::String TypeParamString(int filterNum)
{
    return ParamString("type",filterNum);
}
