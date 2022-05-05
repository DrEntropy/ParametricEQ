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
 
    setSize (800, 600);
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
    const uint scaleAndMeterWidth = 75;
    auto bounds = getLocalBounds();
 
    bounds.reduce(10, 10);
    
    inputMeter.setBounds(bounds.removeFromLeft(scaleAndMeterWidth));
    outputMeter.setBounds(bounds.removeFromRight(scaleAndMeterWidth));
    eqParamContainer.setBounds(bounds.removeFromBottom(100));
     
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
