/*
  ==============================================================================

    Meter.cpp
    Created: 27 Apr 2022 9:53:41am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Meter.h"

//==============================================================================
Meter::Meter()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

Meter::~Meter()
{
}

void Meter::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    auto bar {getLocalBounds().toFloat()};
    
    float y = juce::jmap(peakDb, NEGATIVE_INFINITY, MAX_DECIBELS, bar.getHeight(), 0.f);
    bar.setTop(y);
    g.fillRect (bar);
}


void Meter::update(float dbLevel)
{
    peakDb = dbLevel;
    repaint();
}
