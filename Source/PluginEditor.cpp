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
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello EQ!", getLocalBounds(), juce::Justification::centred, 1);
}

void ParametricEQAudioProcessorEditor::resized()
{
    const uint scaleAndMeterWidth = 50;
    const uint meterWidth = 25;
    const uint meterSpacer = 10;
    
    auto bounds = getLocalBounds();
    auto scaledMeterBounds = bounds.removeFromLeft(scaleAndMeterWidth);
    inputScale.setBounds(scaledMeterBounds.removeFromLeft(meterWidth));
    auto meterBounds = scaledMeterBounds.withTrimmedTop(meterSpacer).withTrimmedBottom(meterSpacer);
    
#ifdef TEST_METER
    meterBounds.setY(JUCE_LIVE_CONSTANT(meterBounds.getY()));
    meterBounds.setHeight(JUCE_LIVE_CONSTANT(meterBounds.getHeight()));
#endif
    
    inputMeter.setBounds(meterBounds);
    inputScale.buildBackgroundImage(TICK_INTERVAL, meterBounds, NEGATIVE_INFINITY, MAX_DECIBELS);
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
