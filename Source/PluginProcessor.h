/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

//#define USE_TEST_OSC

#include <JuceHeader.h>
#include "HighCutLowCutParameters.h"
#include "FilterParameters.h"
#include "Fifo.h"
#include "CoefficientsMaker.h"
#include "ParameterHelpers.h"
#include "FilterCoefficientGenerator.h"
#include "ReleasePool.h"
#include "FilterLink.h"
#include "MeterValues.h"
#include "SingleChannelSampleFifo.h"

using Filter = juce::dsp::IIR::Filter<float>;
using Trim = juce::dsp::Gain<float>;
using CutChain = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
using CutFilter = FilterLink<CutChain, CutCoeffArray, HighCutLowCutParameters, CoefficientsMaker>;
using ParametricFilter = FilterLink<Filter, FilterCoeffPtr, FilterParameters, CoefficientsMaker>;

using ParamLayout = juce::AudioProcessorValueTreeState::ParameterLayout;

const float rampTime = 0.05f;  //50 mseconds
const int innerLoopSize = 32;
 
using MonoFilterChain = juce::dsp::ProcessorChain<CutFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            ParametricFilter,
                                            CutFilter>;



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
    
    bool isAnyActiveOn();  // returns true if all channels filteres bypassed, 8 or 16
    
    void setBypassed(bool state);  // sets ALL filters to bypassed or not depending on state
     
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Params", createParameterLayout() };
    
    // Buffers for meters and fft. 30 should be plenty, timer goes at 60 times a second,
    // which is a duration of about 768 samples as 48k, which should only be few blocks.  
    Fifo<juce::AudioBuffer<float>, 30> inputBuffers;
    Fifo<MeterValues, 30> inMeterValuesFifo, outMeterValuesFifo;
    
    SingleChannelSampleFifo<juce::AudioBuffer<float>>;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
    
    void setBoolParamState(bool state, juce::AudioParameterBool* param);
    
    template<typename T, typename U>
    void updateMeterFifos(T& fifo, U& buffer)
    {
        MeterValues values;
        
        values.leftPeakDb.setGain(buffer.getMagnitude(0, 0, buffer.getNumSamples()));
        values.rightPeakDb.setGain(buffer.getMagnitude(1, 0, buffer.getNumSamples()));
        values.leftRmsDb.setGain(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        values.rightRmsDb.setGain(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        
        fifo.push(values);
    }
    
    template <const int filterNum>
    FilterParameters getParametericFilterParams(Channel channel, double sampleRate)
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
    HighCutLowCutParameters getCutFilterParams(Channel channel, double sampleRate,bool isLowCut)
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
    
    template <const int filterNum>
    void preUpdateParametricFilter(ChannelMode mode, double sampleRate)
    {
        FilterParameters parametricParamsLeft = getParametericFilterParams<filterNum>(Channel::Left, sampleRate);
        FilterParameters parametricParamsRight;
        
        if(mode == ChannelMode::Stereo)
            parametricParamsRight = parametricParamsLeft;
        else
            parametricParamsRight = getParametericFilterParams<filterNum>(Channel::Right, sampleRate);
        
        leftChain.get<filterNum>().performPreloopUpdate(parametricParamsLeft);
        rightChain.get<filterNum>().performPreloopUpdate(parametricParamsRight);
    }
    
    
    template <const int filterNum>
    void loopUpdateParametricFilter(int samplesToSkip)
    {
        leftChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
        rightChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
    }
    
    template <const int filterNum>
    void preUpdateCutFilter(ChannelMode mode, double sampleRate, bool isLowCut)
    {
        HighCutLowCutParameters cutParamsLeft = getCutFilterParams<filterNum>(Channel::Left, sampleRate, isLowCut);
        HighCutLowCutParameters cutParamsRight;
        
        if(mode == ChannelMode::Stereo)
            cutParamsRight = cutParamsLeft;
        else
            cutParamsRight = getCutFilterParams<filterNum>(Channel::Right, sampleRate, isLowCut);
            
        leftChain.get<filterNum>().performPreloopUpdate(cutParamsLeft);
        rightChain.get<filterNum>().performPreloopUpdate(cutParamsRight);
   
    }
    
    template <const int filterNum>
    void loopUpdateCutFilter(int samplesToSkip)
    {
        leftChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
        rightChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
    }
    
    
    template <const int filterNum, typename ParamType>
    void initializeChain(ParamType params, bool onRealTimeThread, double sampleRate)
    {
        leftChain.get<filterNum>().initialize(params, rampTime, onRealTimeThread, sampleRate);
        rightChain.get<filterNum>().initialize(params, rampTime, onRealTimeThread, sampleRate);
    }
    
    void initializeFilters(Channel channel, double sampleRate);
    void performInnerLoopUpdate(int samplesToSkip);
    void performPreLoopUpdate(ChannelMode mode, double sampleRate);
    void updateTrims();
    
    void addFilterParamToLayout(ParamLayout&, Channel, int, bool);
    void createFilterLayouts(ParamLayout& layout, Channel channel);
    
    void performMidSideTransform(juce::AudioBuffer<float>&);
 
    ParamLayout createParameterLayout();
    MonoFilterChain leftChain, rightChain;
    Trim inputTrim, outputTrim;
    
#ifdef USE_TEST_OSC
    juce::dsp::Oscillator<float> testOsc {[] (float x) { return std::sin (x); } , 128};
    juce::dsp::Gain<float> testOscGain;
#endif
    
};
