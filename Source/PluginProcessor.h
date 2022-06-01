/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#define USE_TEST_OSC false
#define USE_WHITE_NOISE false


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
#include "FFTDataGenerator.h"
#include "AnalyzerProperties.h"
#include "ChainHelpers.h"

#define SCSF_SIZE 2048

 
using Trim = juce::dsp::Gain<float>;
 
using ParamLayout = juce::AudioProcessorValueTreeState::ParameterLayout;

const float rampTime = 0.05f;  //50 mseconds
const int innerLoopSize = 32;
 


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
    
    struct SampleRateListener
    {
        virtual ~SampleRateListener() = default;
        virtual void sampleRateChanged(double sr) = 0;
    };

    
    void addSampleRateListener (SampleRateListener*);
    void removeSampleRateListener (SampleRateListener*);
    
  
     
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Params", createParameterLayout() };
    
    // Buffers for meters and fft. 30 should be plenty, timer goes at 60 times a second,
    // which is a duration of about 768 samples as 48k, which should only be few blocks.  
    //Fifo<juce::AudioBuffer<float>, 30> inputBuffers;
    Fifo<MeterValues, 30> inMeterValuesFifo, outMeterValuesFifo;
    
    SingleChannelSampleFifo<juce::AudioBuffer<float>>  leftSCSFifo{Channel::Left}, rightSCSFifo{Channel::Right};
    
    bool editorActive {false};
    
#if USE_TEST_OSC
    std::atomic<size_t> binNum {1};
#endif
    


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
    
    
    template <const ChainPosition chainPos>
    void preUpdateParametricFilter(ChannelMode mode, double sampleRate)
    {
        using namespace ChainHelpers;
        FilterParameters parametricParamsLeft = getParametericFilterParams(chainPos, Channel::Left, sampleRate, apvts);
        FilterParameters parametricParamsRight;
        
        if(mode == ChannelMode::Stereo)
            parametricParamsRight = parametricParamsLeft;
        else
            parametricParamsRight = getParametericFilterParams(chainPos, Channel::Right, sampleRate, apvts);
        
        constexpr int filterNum = static_cast<int>(chainPos);
        leftChain.get<filterNum>().performPreloopUpdate(parametricParamsLeft);
        rightChain.get<filterNum>().performPreloopUpdate(parametricParamsRight);
    }
    
    
    template <const ChainPosition chainPos>
    void loopUpdateParametricFilter(int samplesToSkip)
    {
        constexpr int filterNum = static_cast<int>(chainPos);
        leftChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
        rightChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
    }
    
    template <const ChainPosition chainPos>
    void preUpdateCutFilter(ChannelMode mode, double sampleRate)
    {
        using namespace ChainHelpers;
        HighCutLowCutParameters cutParamsLeft = getCutFilterParams(chainPos, Channel::Left, sampleRate, apvts);
        HighCutLowCutParameters cutParamsRight;
        
        if(mode == ChannelMode::Stereo)
            cutParamsRight = cutParamsLeft;
        else
            cutParamsRight = getCutFilterParams(chainPos, Channel::Right, sampleRate, apvts);
        
        constexpr int filterNum = static_cast<int>(chainPos);
        leftChain.get<filterNum>().performPreloopUpdate(cutParamsLeft);
        rightChain.get<filterNum>().performPreloopUpdate(cutParamsRight);
    }
    
    template <const ChainPosition chainPos>
    void loopUpdateCutFilter(int samplesToSkip)
    {
        constexpr int filterNum = static_cast<int>(chainPos);
        leftChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
        rightChain.get<filterNum>().performInnerLoopFilterUpdate(true, samplesToSkip);
    }
    
    
   
    
    void initializeFilters(ChainHelpers::MonoFilterChain& chain, Channel channel, double sampleRate);
    void performInnerLoopUpdate(int samplesToSkip);
    void performPreLoopUpdate(ChannelMode mode, double sampleRate);
    void updateTrims();
    
    
    void addFilterParamToLayout(ParamLayout&, Channel, ChainPosition, bool);
    void createFilterLayouts(ParamLayout& layout, Channel channel);
    
    void performMidSideTransform(juce::AudioBuffer<float>&);
 
    ParamLayout createParameterLayout();
    ChainHelpers::MonoFilterChain leftChain, rightChain;
    Trim inputTrim, outputTrim;
    
    juce::ListenerList<SampleRateListener> sampleRateListeners;
    
#if USE_TEST_OSC
    juce::dsp::Oscillator<float> testOsc {[] (float x) { return std::sin (x); }};
    juce::dsp::Gain<float> testOscGain;
#endif
    
#if USE_WHITE_NOISE
    juce::Random random;
#endif
    
};
