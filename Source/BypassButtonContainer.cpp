/*
  ==============================================================================

    BypassButtonContainer.cpp
    Created: 9 May 2022 11:44:53am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BypassButtonContainer.h"

//==============================================================================
BypassButtonContainer::BypassButtonContainer(juce::AudioProcessorValueTreeState& apvts)
{
    int filterNum = 0;
    for(auto& buttonp : bypassButtons)
    {
        buttonp.reset(new DualBypassButton(static_cast<ChainPosition> (filterNum), apvts));
        addAndMakeVisible(*buttonp);
        filterNum++;
    }
}
 

void BypassButtonContainer::paint (juce::Graphics& g)
{
   // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}



void BypassButtonContainer::resized()
{
 
    auto height= getLocalBounds().getHeight();
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.justifyContent  = juce::FlexBox::JustifyContent::spaceAround;
    for (auto& b : bypassButtons)
         fb.items.add (juce::FlexItem (*b).withMinWidth(height*2).withMinHeight(height));
     
    fb.performLayout (getLocalBounds().toFloat());
}
