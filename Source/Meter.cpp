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
Meter::Meter(juce::String aLabel) : chanLabel {aLabel}
{
    decayingValueHolder.setDecayRate(3.0f);
}

Meter::~Meter()
{
}

void Meter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto labelBounds = bounds.removeFromTop(labelSize).toNearestInt();
    
    g.setColour(juce::Colours::black);
    g.fillRect(bounds);
    
    paintBar(g, peakDb, bounds, 0, juce::Colours::darkgoldenrod);
    paintBar(g, averageDb.getAvg(), bounds, bounds.getWidth()/4.0f, juce::Colours::gold);

    if (decayingValueHolder.isOverThreshold())
        g.setColour(juce::Colours::red);
    else
        g.setColour(juce::Colours::orange);
    
    float decayBarY =juce::jmap(decayingValueHolder.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS,bounds.getY() + bounds.getHeight(), bounds.getY());
    g.fillRect(0.f, std::max(0.f,decayBarY - DECAY_BAR_THICK/2), bounds.getWidth(), DECAY_BAR_THICK);
     
    g.drawFittedText(chanLabel, labelBounds, juce::Justification::centred, 1);
}

void Meter::paintBar (juce::Graphics& g, float value, juce::Rectangle<float> bounds, float dWidth, juce::Colour color)
{
    auto bar { bounds };
    
    g.setColour(color);
    float y = juce::jmap(value, NEGATIVE_INFINITY, MAX_DECIBELS, bar.getY() + bar.getHeight(), bar.getY());
    bar.setTop(y);
    bar.reduce(dWidth, 0);
    g.fillRect(bar);
}


juce::Rectangle<int> Meter::getMeterBounds()
{
    return getBounds().withTrimmedTop(labelSize);
}

void Meter::update(float dbLevelPeak, float dbLevelRMS)
{
    peakDb = dbLevelPeak;
    averageDb.add(dbLevelRMS);
    decayingValueHolder.updateHeldValue(peakDb);
    repaint();
}

 
