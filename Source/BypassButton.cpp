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
  
    auto bounds = getLocalBounds().toFloat();
    bounds.removeFromTop(SINGLE_BYPASS_MARGIN);
    bounds.removeFromBottom(SINGLE_BYPASS_MARGIN);
   
    if(isShowingAsOn())
    {
        if(onLeft)
            bounds.removeFromLeft(SINGLE_BYPASS_MARGIN);
        
        if(!onLeft || !isPaired)
            bounds.removeFromRight(SINGLE_BYPASS_MARGIN);
 
        g.setColour(juce::Colours::green);
        g.fillRect(bounds);
    }
     
}
