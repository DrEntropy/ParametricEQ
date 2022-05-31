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

template <const int filterNum>
FilterParameters getParametericFilterParams(Channel channel, double sampleRate, juce::AudioProcessorValueTreeState& apvts)
{
    using namespace FilterInfo;
    
    float frequency = apvts.getRawParameterValue(createFreqParamString(channel, filterNum))->load();
    float quality  = apvts.getRawParameterValue(createQParamString(channel, filterNum))->load();
    bool bypassed = apvts.getRawParameterValue(createBypassParamString(channel, filterNum))->load() > 0.5f;
    
    FilterParameters parametricParams;
    
    
    switch(filterNum)
    {
        case 1:
            parametricParams.filterType =  FilterType::LowShelf;
            break;
        case 6:
            parametricParams.filterType = FilterType::HighShelf;
            break;
        default:
            parametricParams.filterType = FilterType::PeakFilter;
    }
        
    
    parametricParams.frequency = frequency;
    parametricParams.sampleRate = sampleRate;
    parametricParams.quality = quality;
    parametricParams.bypassed = bypassed;
    parametricParams.gain = Decibel <float> (apvts.getRawParameterValue(createGainParamString(channel, filterNum))-> load());
    
    return parametricParams;

}

template <const int filterNum>
HighCutLowCutParameters getCutFilterParams(Channel channel, double sampleRate,bool isLowCut, juce::AudioProcessorValueTreeState& apvts)
{
    using namespace FilterInfo;
    
    float frequency = apvts.getRawParameterValue(createFreqParamString(channel, filterNum))->load();
    float quality  = apvts.getRawParameterValue(createQParamString(channel, filterNum))->load();
    bool bypassed = apvts.getRawParameterValue(createBypassParamString(channel, filterNum))->load() > 0.5f;
    
    Slope slope = static_cast<Slope> (apvts.getRawParameterValue(createSlopeParamString(channel, filterNum))->load());
    
    HighCutLowCutParameters cutParams;
        
    cutParams.isLowcut = isLowCut;
    cutParams.frequency = frequency;
    cutParams.bypassed = bypassed;
    cutParams.order = static_cast<int>(slope) + 1;
    cutParams.sampleRate = sampleRate;
    cutParams.quality  = quality;
    
    return cutParams;
}

template <const int filterNum, typename ParamType>
void initializeChain(MonoFilterChain& chain, ParamType params,float rampTime, bool onRealTimeThread, double sampleRate)
{
    chain.get<filterNum>().initialize(params, rampTime, onRealTimeThread, sampleRate);
}


}
