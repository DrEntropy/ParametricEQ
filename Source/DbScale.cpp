/*
  ==============================================================================

    DbScale.cpp
    Created: 28 Apr 2022 2:39:51pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DbScale.h"

//==============================================================================
 


void DbScale::paint (juce::Graphics& g)
{
    g.drawImage(bkgd, getLocalBounds().toFloat());
}
 
void DbScale::buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    jassert(minDb <= maxDb);
    
    auto bounds = getLocalBounds();
    
    if(bounds.isEmpty())
        return;
    
    auto scaleFactor = juce::Desktop::getInstance().getDisplays().getDisplayForPoint({0,0}) -> scale;
    bkgd = juce::Image(juce::Image::RGB, bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor, true);
    
    juce::Graphics g(bkgd);
    
    g.addTransform(juce::AffineTransform().scale(scaleFactor));
    
    g.setColour(juce::Colours::red);
    g.drawRect(bounds);
    
}

std::vector<Tick> DbScale::getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    
}
