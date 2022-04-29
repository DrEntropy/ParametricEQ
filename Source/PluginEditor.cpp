/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(inputScale);
 
    setSize (800, 600);
    startTimerHz(60);  
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello EQ!", getLocalBounds(), juce::Justification::centred, 1);
}

void ParametricEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto scaledMeterBounds = bounds.removeFromLeft(50);
    inputScale.setBounds(scaledMeterBounds.removeFromLeft(25));
    auto meterBounds = scaledMeterBounds.withTrimmedTop(10).withTrimmedBottom(10);
    inputMeter.setBounds(meterBounds);
    inputScale.buildBackgroundImage(0, meterBounds, NEGATIVE_INFINITY, MAX_DECIBELS);
}


void ParametricEQAudioProcessorEditor::timerCallback()
{
    auto& inputFifo = audioProcessor.inputBuffers;
    if(inputFifo.getNumAvailableForReading() > 0)
    {
        while(inputFifo.pull(buffer))
        {
            // nothing ES.85
        }
        auto magnitude = buffer.getMagnitude(0, 0, buffer.getNumSamples());
        inputMeter.update(juce::Decibels::gainToDecibels(magnitude, NEGATIVE_INFINITY));
    }
}
