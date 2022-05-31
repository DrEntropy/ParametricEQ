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

enum class Channel { Left, Right};

enum class ChannelMode {Stereo, DualMono, MidSide};

const std::map<ChannelMode,juce::String>  mapModeToString
{
    {ChannelMode::Stereo, "Stereo"},
    {ChannelMode::DualMono, "DualMono"},
    {ChannelMode::MidSide, "Mid-Side"},
};

using ChainPosition = int;

juce::String createGainParamString(Channel channel, ChainPosition filterNum);

juce::String createQParamString(Channel channel, ChainPosition filterNum);

juce::String createFreqParamString(Channel channel, ChainPosition filterNum);

juce::String createBypassParamString(Channel channel, ChainPosition filterNum);

juce::String createTypeParamString(Channel channel, ChainPosition filterNum);

juce::String createSlopeParamString(Channel channel, ChainPosition filterNum);

juce::String createChannelString(Channel channel);
 
