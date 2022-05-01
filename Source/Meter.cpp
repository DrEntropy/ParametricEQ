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
    auto bounds = getLocalBounds().toFloat();
    
    g.fillAll(juce::Colours::black);
    
    paintBar(g, peakDb, bounds, 0, juce::Colours::darkgoldenrod);
    paintBar(g, averageDb.getAvg(), bounds, bounds.getWidth()/4.0f, juce::Colours::gold);

    if (decayingValueHolder.isOverThreshold())
        g.setColour(juce::Colours::red);
    else
        g.setColour(juce::Colours::orange);
    
    float decayBarY =juce::jmap(decayingValueHolder.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS, bounds.getHeight(), 0.f);
    g.fillRect(0.f, std::max(0.f,decayBarY - DECAY_BAR_THICK/2), bounds.getWidth(), DECAY_BAR_THICK);
}

void Meter::paintBar (juce::Graphics& g, float value, juce::Rectangle<float> bounds, float dWidth, juce::Colour color)
{
    auto bar { bounds };
    
    g.setColour(color);
    float y = juce::jmap(value, NEGATIVE_INFINITY, MAX_DECIBELS, bar.getHeight(), 0.f);
    bar.setTop(y);
    bar.reduce(dWidth, 0);
    g.fillRect(bar);
}


void Meter::update(float dbLevel)
{
    peakDb = dbLevel;
    averageDb.add(peakDb);
    decayingValueHolder.updateHeldValue(peakDb);
    repaint();
}
