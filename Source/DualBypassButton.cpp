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
DualBypassButton::DualBypassButton(int filterNum, juce::AudioProcessorValueTreeState& apvts):filterNum(filterNum), apvts(apvts)
{
    addAndMakeVisible(leftMidBypass);
    addAndMakeVisible(rightSideBypass);
 
    leftMidAttachment.reset(new ButtonAttachment(apvts, createBypassParamString(Channel::Left, filterNum), leftMidBypass));
    rightSideAttachment.reset(new ButtonAttachment(apvts, createBypassParamString(Channel::Right, filterNum), rightSideBypass));
    
    auto safePtr = juce::Component::SafePointer<DualBypassButton>(this);
    modeListener.reset(new ParamListener(apvts.getParameter("Processing Mode"),
                                        [safePtr](float v)
                                         {
                                          if(auto* comp = safePtr.getComponent() )
                                              comp->refreshButtons(static_cast<ChannelMode>(v));
                                         }));
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

void DualBypassButton::paintOverChildren(juce::Graphics& g)
{
    g.drawFittedText("testing", getLocalBounds(), juce::Justification::centred, 1);
}

void DualBypassButton::resized()
{
    refreshButtons(static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load()));
}


void DualBypassButton::refreshButtons(ChannelMode mode)
{
    auto bounds = getLocalBounds();
    
    if(mode == ChannelMode::Stereo)
    {
        rightSideBypass.setVisible(false);
        leftMidBypass.setBounds(bounds);
    }
    else
    {
        rightSideBypass.setVisible(true);
        leftMidBypass.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
        rightSideBypass.setBounds(bounds);
        
    }
}
