/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HighCutLowCutParameters.h"
#include "FilterParameters.h"
#include "Fifo.h"
#include "CoefficientsMaker.h"
#include "ParameterHelpers.h"
#include "FilterCoefficientGenerator.h"
#include "ReleasePool.h"
#include "FilterLink.h"

using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
using CutFilterLink = FilterLink<CutFilter, CutCoeffArray, HighCutLowCutParameters, CoefficientsMaker>;
using ParametricFilter = FilterLink<Filter, FilterCoeffPtr, FilterParameters, CoefficientsMaker>;
 
using MonoChain = juce::dsp::ProcessorChain<CutFilterLink,ParametricFilter,CutFilterLink>;

//==============================================================================
/**
*/
class ParametricEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ParametricEQAudioProcessor();
    ~ParametricEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // =========================================================================
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Params", createParameterLayout() };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
    
    template <const int filterNum>
    FilterParameters getParametericFilterParams(double sampleRate)
    {
        using namespace FilterInfo;
        
        float frequency = apvts.getRawParameterValue(createFreqParamString(filterNum))->load();
        float quality  = apvts.getRawParameterValue(createQParamString(filterNum))->load();
        bool bypassed = apvts.getRawParameterValue(createBypassParamString(filterNum))->load() > 0.5f;
        
        FilterType filterType = static_cast<FilterType> (apvts.getRawParameterValue(createTypeParamString(filterNum))->load());
        
        
        FilterParameters parametricParams;
        
        parametricParams.frequency = frequency;
        parametricParams.filterType = filterType;
        parametricParams.sampleRate = sampleRate;
        parametricParams.quality = quality;
        parametricParams.bypassed = bypassed;
        parametricParams.gain = Decibel <float> (apvts.getRawParameterValue(createGainParamString(filterNum))-> load());
        
        return parametricParams;
        
    }
    
    template <const int filterNum>
    HighCutLowCutParameters getCutFilterParams(double sampleRate,bool isLowCut)
    {
        using namespace FilterInfo;
        
        float frequency = apvts.getRawParameterValue(createFreqParamString(filterNum))->load();
        bool bypassed = apvts.getRawParameterValue(createBypassParamString(filterNum))->load() > 0.5f;
        
        Slope slope = static_cast<Slope> (apvts.getRawParameterValue(createSlopeParamString(filterNum))->load());
        
        HighCutLowCutParameters cutParams;
            
        cutParams.isLowcut = isLowCut;
        cutParams.frequency = frequency;
        cutParams.bypassed = bypassed;
        cutParams.order = static_cast<int>(slope) + 1;
        cutParams.sampleRate = sampleRate;
        cutParams.quality  = 1.0f; //not used for cut filters
        
        return cutParams;
        
    }
    
    template <const int filterNum>
    void updateParametricFilter(double sampleRate)
    {
 
        FilterParameters parametricParams = getParametericFilterParams<filterNum>(sampleRate);
        
        leftChain.get<filterNum>().performPreloopUpdate(parametricParams);
        leftChain.get<filterNum>().performInnerLoopFilterUpdate(true,0);
        rightChain.get<filterNum>().performPreloopUpdate(parametricParams);
        rightChain.get<filterNum>().performInnerLoopFilterUpdate(true,0);
        
        
    }
    
    template <const int filterNum>
    void updateCutFilter(double sampleRate, bool isLowCut)
    {
 
        
        HighCutLowCutParameters cutParams = getCutFilterParams<filterNum>(sampleRate, isLowCut);
            
        leftChain.get<filterNum>().performPreloopUpdate(cutParams);
        leftChain.get<filterNum>().performInnerLoopFilterUpdate(true,0);
        rightChain.get<filterNum>().performPreloopUpdate(cutParams);
        rightChain.get<filterNum>().performInnerLoopFilterUpdate(true,0);
        
    }
    
     
    
  
    void initializeFilters(double sampleRate);
    void updateFilters(double sampleRate, bool forceUpdate = false);
    
    using ParamLayout = juce::AudioProcessorValueTreeState::ParameterLayout;
    
    void addFilterParamToLayout(ParamLayout&, int,bool);
 
    ParamLayout createParameterLayout();
    MonoChain leftChain, rightChain;
    

    
};
