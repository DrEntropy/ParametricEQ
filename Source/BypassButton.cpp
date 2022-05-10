/*
  ==============================================================================

    BypassButton.cpp
    Created: 9 May 2022 11:43:57am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "BypassButton.h"


BypassButton::BypassButton() : Button("")
{
    setClickingTogglesState(true);
}


void BypassButton::paintButton (juce::Graphics& g,bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    // this is all just placeholder
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::darkblue);
    if(isShowingAsOn())
    {
        g.setColour(juce::Colours::green);
    }
    
    g.fillRect(bounds);
    g.setColour(juce::Colours::darkblue);
    g.drawRect(bounds, 2);
}
