/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MeterValues.h"

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(eqParamContainer);
    
    addAndMakeVisible(bypassButtonContainer);
    
    addAndMakeVisible(globalBypass);
 
    setSize (1200, 800);
    startTimerHz(FRAME_RATE);
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("PFM11-23", getLocalBounds(), juce::Justification::centred, 1);
}

void ParametricEQAudioProcessorEditor::resized()
{
    // to do, move magic numbers to a common spot
    
//#define BYPASS_SWITCH_HEIGHT 50
//#define BYPASS_SWITCH_V_MARGIN 5
////#define GLOBAL_SWITCH_RIGHT_MARGIN 15
//    PARAM_CONTROLS_MARGIN 4
//   #define PARAM_CONTROLS_HEIGHT 100
 
    auto bounds = getLocalBounds();
    
    auto bottomBounds = bounds.removeFromBottom(BOTTOM_CONTROLS_HEIGHT); // placeholder for bottom controls
 
    bounds.reduce(OVERALL_MARGIN, OVERALL_MARGIN);
    
    inputMeter.setBounds(bounds.removeFromLeft(SCALE_AND_METER_WIDTH));
    outputMeter.setBounds(bounds.removeFromRight(SCALE_AND_METER_WIDTH));
    
    //  These magic numbers are placeholders
    eqParamContainer.setBounds(bounds.removeFromBottom(PARAM_CONTROLS_HEIGHT).reduced(PARAM_CONTROLS_MARGIN));
    
    auto topBounds = bounds.removeFromTop(BYPASS_SWITCH_HEIGHT + 2 * BYPASS_SWITCH_V_MARGIN);

    bypassButtonContainer.setBounds(bounds.removeFromTop(BYPASS_SWITCH_HEIGHT));
    
    globalBypass.setBounds(topBounds.removeFromTop(BYPASS_SWITCH_HEIGHT + BYPASS_SWITCH_V_MARGIN).withTrimmedBottom(BYPASS_SWITCH_V_MARGIN)
                                    .withTrimmedRight(GLOBAL_SWITCH_RIGHT_MARGIN).removeFromRight(2 * BYPASS_SWITCH_HEIGHT));
     
}


void ParametricEQAudioProcessorEditor::timerCallback()
{
    auto& inputFifo = audioProcessor.inMeterValuesFifo;
    auto& outputFifo = audioProcessor.outMeterValuesFifo;
    
    MeterValues values;
    
    if(inputFifo.getNumAvailableForReading() > 0)
    {
        while(inputFifo.pull(values))
        {
            // nothing  
        }
        inputMeter.update(values);
    }
    
    if(outputFifo.getNumAvailableForReading() > 0)
    {
        while(outputFifo.pull(values))
        {
            // nothing ES.85
        }
        outputMeter.update(values);
    }
}
