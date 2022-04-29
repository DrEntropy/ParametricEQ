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
    decayingValueHolder.setDecayRate(3.0f);
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
    auto bounds = getLocalBounds().toFloat();
    auto bar { bounds };
    
    float y = juce::jmap(peakDb, NEGATIVE_INFINITY, MAX_DECIBELS, bar.getHeight(), 0.f);
    bar.setTop(y);
    g.fillRect (bar);
    
    if (decayingValueHolder.isOverThreshold())
        g.setColour(juce::Colours::red);
    else
        g.setColour(juce::Colours::orange);
    
    float decayBarY =juce::jmap(decayingValueHolder.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS, bounds.getHeight(), 0.f);
    g.fillRect(0.f, std::max(0.f,decayBarY - DECAY_BAR_THICK/2), bounds.getWidth(), DECAY_BAR_THICK);
}


void Meter::update(float dbLevel)
{
    peakDb = dbLevel;
    decayingValueHolder.updateHeldValue(peakDb);
    repaint();
}
