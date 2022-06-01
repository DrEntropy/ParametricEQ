/*
  ==============================================================================

    ParameterHelpers.cpp
    Created: 3 Apr 2022 2:14:53pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "ParameterHelpers.h"

juce::String createFilterNumberString(ChainPosition chainpos)
{
    return juce::String(static_cast<int>(chainpos));
}

juce::String createChannelString(Channel channel)
{
   if (channel == Channel::Left)
       return "left_";
    return "right_";
}

juce::String createParamString(Channel channel, juce::String label, ChainPosition filterNum)
{
    return "Filter_" + createChannelString(channel) + createFilterNumberString(filterNum)+"_"+ label;
}


juce::String createGainParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "gain",filterNum);
}

juce::String createQParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "Q",filterNum);
}

juce::String createFreqParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "freq",filterNum);
}

juce::String createBypassParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "bypass",filterNum);
}

juce::String createTypeParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "type",filterNum);
}


juce::String createSlopeParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "slope",filterNum);
}
