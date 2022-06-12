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

enum class ChainPosition
{
    LowCut,
    LowShelf,
    PeakFilter1,
    PeakFilter2,
    PeakFilter3,
    PeakFilter4,
    HighShelf,
    HighCut
};

juce::String createFilterNumberString(ChainPosition chainpos);

juce::String createGainParamString(Channel channel, ChainPosition chainpos);

juce::String createQParamString(Channel channel, ChainPosition chainpos);

juce::String createFreqParamString(Channel channel, ChainPosition chainpos);

juce::String createBypassParamString(Channel channel, ChainPosition chainpos);

juce::String createTypeParamString(Channel channel, ChainPosition chainpos);

juce::String createSlopeParamString(Channel channel, ChainPosition chainpos);

juce::String createChannelString(Channel channel);
 
juce::RangedAudioParameter* getFrequencyParam(juce::AudioProcessorValueTreeState& apvts, Channel channel, ChainPosition chainpos);

juce::RangedAudioParameter* getQParam(juce::AudioProcessorValueTreeState& apvts, Channel channel, ChainPosition chainpos);

juce::RangedAudioParameter* getGainOrSlopeParam(juce::AudioProcessorValueTreeState& apvts, Channel channel, ChainPosition chainpos);
