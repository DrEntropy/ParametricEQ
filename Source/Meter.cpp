/*
  ==============================================================================

    Meter.cpp
    Created: 27 Apr 2022 9:53:41am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Meter.h"
#include "DbScale.h"

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
    auto labelBounds = bounds.removeFromTop(LABEL_SIZE).toNearestInt();
    
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
    
    auto scaleBounds = bounds.reduced(INNER_TICK_SHRINK, 0);
    paintInnerTicks(g, scaleBounds, juce::Colours::lightgrey);
}

void Meter::paintInnerTicks(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color)
{
    
    g.setColour(color);
    auto ticks = DbScale::getTicks(INNER_TICK_INTERVAL, bounds.toNearestInt(), NEGATIVE_INFINITY, MAX_DECIBELS);
    
    size_t i=0;
    for(auto tick : ticks)
    {
        if( i != 0 && i < ticks.size() - 1)
        {
            juce::Rectangle<float> tickMark {bounds.getX(), tick.y-INNER_TICK_THICK/2,  bounds.getWidth(), INNER_TICK_THICK};
            g.fillRect(tickMark);
        }
        i++;
    }
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
    return getBounds().withTrimmedTop(LABEL_SIZE);
}

void Meter::update(float dbLevelPeak, float dbLevelRMS)
{
    peakDb = dbLevelPeak;
    averageDb.add(dbLevelRMS);
    decayingValueHolder.updateHeldValue(peakDb);
    repaint();
}

 
