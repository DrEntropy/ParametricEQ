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
    
}

std::vector<Tick> DbScale::getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    
}
