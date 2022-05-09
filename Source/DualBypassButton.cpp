/*
  ==============================================================================

    DualBypassButton.cpp
    Created: 9 May 2022 11:44:33am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DualBypassButton.h"
#include "ParameterHelpers.h"

//==============================================================================
DualBypassButton::DualBypassButton(int filterNum, juce::AudioProcessorValueTreeState& apvts):filterNum(filterNum)
{
    addAndMakeVisible(leftMidBypass);
    addAndMakeVisible(rightSideBypass);
    
    leftMidAttachment.reset(new ButtonAttachment(apvts, createBypassParamString(Channel::Left, filterNum), leftMidBypass));
    rightSideAttachment.reset(new ButtonAttachment(apvts, createBypassParamString(Channel::Right, filterNum), rightSideBypass));
}



void DualBypassButton::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

}

void DualBypassButton::resized()
{
    auto bounds = getLocalBounds();
    leftMidBypass.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
    rightSideBypass.setBounds(bounds);
    

}
