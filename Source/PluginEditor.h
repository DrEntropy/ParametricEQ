/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

 

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "StereoMeter.h"
#include "ParamListener.h"
#include "EQParamContainer.h"
#include "BypassButtonContainer.h"
#include "GlobalBypass.h"


//layout defines
#define OVERALL_MARGIN 10
#define BYPASS_SWITCH_HEIGHT 50
#define BYPASS_SWITCH_V_MARGIN 5
#define GLOBAL_SWITCH_RIGHT_MARGIN 15
#define SCALE_AND_METER_WIDTH 75
#define BOTTOM_CONTROLS_HEIGHT 100

#define PARAM_CONTROLS_MARGIN 4
#define PARAM_CONTROLS_HEIGHT 100

//==============================================================================
/**
*/
class ParametricEQAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor&);
    ~ParametricEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ParametricEQAudioProcessor& audioProcessor;
    
    juce::AudioBuffer<float> buffer;
    StereoMeter inputMeter {"PRE EQ"};
    StereoMeter outputMeter {"POST EQ"};
    
    EQParamContainer eqParamContainer {audioProcessor.apvts};
    
    BypassButtonContainer bypassButtonContainer {audioProcessor.apvts};
    
    GlobalBypass globalBypass {audioProcessor};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessorEditor)
};
