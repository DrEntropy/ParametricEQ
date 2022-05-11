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
    
    rightSideBypass.onLeft = false;
    leftMidBypass.onLeft = true;
    rightSideBypass.isPaired = true;  //never on its own.
 
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



void DualBypassButton::paintOverChildren(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::lightblue);  // note need to draw different colors depending on button state!
    g.drawFittedText("testing", getLocalBounds(), juce::Justification::centred, 1);
    g.drawRect(bounds, 2);
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
        isPaired = false;
        leftMidBypass.isPaired = false;
        rightSideBypass.setVisible(false);
        leftMidBypass.setBounds(bounds);
    }
    else
    {
        isPaired = true;
        leftMidBypass.isPaired = true;
        rightSideBypass.setVisible(true);
        leftMidBypass.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
        rightSideBypass.setBounds(bounds);
    }
}
