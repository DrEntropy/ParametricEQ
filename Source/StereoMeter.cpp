/*
  ==============================================================================

    StereoMeter.cpp
    Created: 2 May 2022 1:31:26pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StereoMeter.h"

//==============================================================================
StereoMeter::StereoMeter(juce::String label) : meterLabel{label}
{
    addAndMakeVisible(leftMeter);
    addAndMakeVisible(rightMeter);
    addAndMakeVisible(dBScale);
}


void StereoMeter::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
  
    auto bounds = getLocalBounds();
 
    auto nameBounds = bounds.removeFromBottom(LABEL_SPACE);
    
    g.setColour(juce::Colours::white);
    g.drawFittedText(meterLabel, nameBounds, juce::Justification::centred, 1);
}

void StereoMeter::resized()
{
    auto bounds = getLocalBounds();
   
    leftMeter.setBounds(bounds.removeFromLeft(METER_WIDTH).withTrimmedBottom(LABEL_SPACE));
    rightMeter.setBounds(bounds.removeFromRight(METER_WIDTH).withTrimmedBottom(LABEL_SPACE));
 
    dBScale.setBounds(bounds);
    dBScale.buildBackgroundImage(TICK_INTERVAL, rightMeter.getMeterBounds(), NEGATIVE_INFINITY, MAX_DECIBELS);
}


void StereoMeter::update(MeterValues values)
{
    leftMeter.update(values.leftPeakDb.getDb(), values.leftRmsDb.getDb());
    rightMeter.update(values.rightPeakDb.getDb(), values.rightRmsDb.getDb());
}
