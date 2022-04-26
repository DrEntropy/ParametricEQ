/*
  ==============================================================================

    ParameterHelpers.cpp
    Created: 3 Apr 2022 2:14:53pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "ParameterHelpers.h"

juce::String createChannelString(Channel channel)
{
   if (channel == Channel::Left)
       return "left_";
    return "right_";
}

juce::String createParamString(Channel channel, juce::String label, int filterNum)
{
    return "Filter_" + createChannelString(channel) + juce::String(filterNum)+"_"+ label;
}


juce::String createGainParamString(Channel channel, int filterNum)
{
    return createParamString(channel, "gain",filterNum);
}

juce::String createQParamString(Channel channel, int filterNum)
{
    return createParamString(channel, "Q",filterNum);
}

juce::String createFreqParamString(Channel channel, int filterNum)
{
    return createParamString(channel, "freq",filterNum);
}

juce::String createBypassParamString(Channel channel, int filterNum)
{
    return createParamString(channel, "bypass",filterNum);
}

juce::String createTypeParamString(Channel channel, int filterNum)
{
    return createParamString(channel, "type",filterNum);
}


juce::String createSlopeParamString(Channel channel, int filterNum)
{
    return createParamString(channel, "slope",filterNum);
}
