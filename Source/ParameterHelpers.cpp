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

juce::String createParamString(Channel channel, juce::String label, ChainPosition chainpos)
{
    return "Filter_" + createChannelString(channel) + createFilterNumberString(chainpos)+"_"+ label;
}


juce::String createGainParamString(Channel channel, ChainPosition chainpos)
{
    return createParamString(channel, "gain", chainpos);
}

juce::String createQParamString(Channel channel, ChainPosition chainpos)
{
    return createParamString(channel, "Q", chainpos);
}

juce::String createFreqParamString(Channel channel, ChainPosition chainpos)
{
    return createParamString(channel, "freq", chainpos);
}

juce::String createBypassParamString(Channel channel, ChainPosition chainpos)
{
    return createParamString(channel, "bypass", chainpos);
}

juce::String createTypeParamString(Channel channel, ChainPosition chainpos)
{
    return createParamString(channel, "type", chainpos);
}


juce::String createSlopeParamString(Channel channel, ChainPosition filterNum)
{
    return createParamString(channel, "slope",filterNum);
}
