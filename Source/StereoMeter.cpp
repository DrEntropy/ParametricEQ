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
StereoMeter::StereoMeter()
{
    //addAndMakeVisible(leftMeter);
    addAndMakeVisible(rightMeter);
    addAndMakeVisible(dBScale);
}


void StereoMeter::paint (juce::Graphics& g)
{
 
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    // to do, add other paint stuff
 
}

void StereoMeter::resized()
{
    const uint meterWidth = 25;
    const uint meterSpacer = 10;
    
    auto bounds = getLocalBounds();
    
    dBScale.setBounds(bounds.removeFromLeft(meterWidth));
    auto meterBounds = bounds.withTrimmedTop(meterSpacer).withTrimmedBottom(meterSpacer);
    
    rightMeter.setBounds(meterBounds);
    dBScale.buildBackgroundImage(TICK_INTERVAL, meterBounds, NEGATIVE_INFINITY, MAX_DECIBELS);
}


void StereoMeter::update(MeterValues values)
{
    leftMeter.update(values.leftPeakDb.getDb());
    rightMeter.update(values.rightPeakDb.getDb());
}
