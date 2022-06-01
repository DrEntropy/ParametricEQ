/*
  ==============================================================================

    ChainHelpers.h
    Created: 26 May 2022 3:10:44pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FilterInfo.h"
#include "FilterLink.h"
 

namespace ChainHelpers
{
 
using Filter = juce::dsp::IIR::Filter<float>;
using CutChain = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
using CutFilter = FilterLink<CutChain, CutCoeffArray, HighCutLowCutParameters, CoefficientsMaker>;
using ParametricFilter = FilterLink<Filter, FilterCoeffPtr, FilterParameters, CoefficientsMaker>;

using MonoFilterChain = juce::dsp::ProcessorChain<CutFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            CutFilter>;

template <const ChainPosition chainPos>
FilterParameters getParametericFilterParams(Channel channel, double sampleRate, juce::AudioProcessorValueTreeState& apvts)
{
    using namespace FilterInfo;
    
    float frequency = apvts.getRawParameterValue(createFreqParamString(channel, chainPos))->load();
    float quality  = apvts.getRawParameterValue(createQParamString(channel, chainPos))->load();
    bool bypassed = apvts.getRawParameterValue(createBypassParamString(channel, chainPos))->load() > 0.5f;
    
    FilterParameters parametricParams;
    
    
    switch(chainPos)
    {
        case ChainPosition::LowShelf:
            parametricParams.filterType =  FilterType::LowShelf;
            break;
        case ChainPosition::HighShelf:
            parametricParams.filterType = FilterType::HighShelf;
            break;
        default:
            parametricParams.filterType = FilterType::PeakFilter;
    }
        
    
    parametricParams.frequency = frequency;
    parametricParams.sampleRate = sampleRate;
    parametricParams.quality = quality;
    parametricParams.bypassed = bypassed;
    parametricParams.gain = Decibel <float> (apvts.getRawParameterValue(createGainParamString(channel, chainPos))-> load());
    
    return parametricParams;

}

template <const ChainPosition chainPos>
HighCutLowCutParameters getCutFilterParams(Channel channel, double sampleRate,bool isLowCut, juce::AudioProcessorValueTreeState& apvts)
{
    using namespace FilterInfo;
    
    float frequency = apvts.getRawParameterValue(createFreqParamString(channel, chainPos))->load();
    float quality  = apvts.getRawParameterValue(createQParamString(channel, chainPos))->load();
    bool bypassed = apvts.getRawParameterValue(createBypassParamString(channel, chainPos))->load() > 0.5f;
    
    Slope slope = static_cast<Slope> (apvts.getRawParameterValue(createSlopeParamString(channel, chainPos))->load());
    
    HighCutLowCutParameters cutParams;
        
    cutParams.isLowcut = isLowCut;
    cutParams.frequency = frequency;
    cutParams.bypassed = bypassed;
    cutParams.order = static_cast<int>(slope) + 1;
    cutParams.sampleRate = sampleRate;
    cutParams.quality  = quality;
    
    return cutParams;
}

template <const ChainPosition chainPos, typename ParamType>
void initializeChain(MonoFilterChain& chain, ParamType params,float rampTime, bool onRealTimeThread, double sampleRate)
{
    chain.get<static_cast<int>(chainPos)>().initialize(params, rampTime, onRealTimeThread, sampleRate);
}


}
