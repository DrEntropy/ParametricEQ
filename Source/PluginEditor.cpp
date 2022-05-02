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
    const uint scaleAndMeterWidth = 75;
    auto bounds = getLocalBounds();
    
    inputMeter.setBounds(bounds.removeFromLeft(scaleAndMeterWidth));
     
}


void ParametricEQAudioProcessorEditor::timerCallback()
{
    auto& inputFifo = audioProcessor.inMeterValuesFifo;
    auto& outputFifo = audioProcessor.outMeterValuesFifo;
    
    MeterValues inputValues;
    
    if(inputFifo.getNumAvailableForReading() > 0)
    {
        while(inputFifo.pull(inputValues))
        {
            // nothing ES.85
        }
        inputMeter.update(inputValues);
        // TODO this will be done in a helper  and in the plug in processor

        inputMeter.update(inputValues);
    }
}
