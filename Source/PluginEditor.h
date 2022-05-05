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
    
    EQParamContainer eqParamContainer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessorEditor)
};
