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
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("DbScale", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}
 
void DbScale::buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    
}

static std::vector<Tick> DbScale::getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    
}
