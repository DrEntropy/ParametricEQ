/*
  ==============================================================================

    SwitchControl.cpp
    Created: 22 May 2022 3:51:55pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SwitchControl.h"

//==============================================================================
SwitchControl::SwitchControl(juce::AudioProcessorValueTreeState& apvts, juce::String parameterName)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, parameterName, slider));
    auto myParameter = dynamic_cast<juce::AudioParameterChoice*>( apvts.getParameter(parameterName) );
    
    jassert(myParameter);  //switch control should only be used with choice parameters
    
    slider.choices =  myParameter->choices;
    
    addAndMakeVisible(slider);
    label.setText(myParameter->name, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(label);
}

SwitchControl::~SwitchControl()
{
    attachment.reset();
}


void SwitchControl::resized()
{
    auto bounds = getLocalBounds();
    auto w = bounds.getWidth();
    bounds.removeFromLeft(w / 6);
    label.setBounds(bounds.removeFromTop(w / 4));
    bounds.removeFromBottom(w / 6);
    
    slider.setBounds(bounds);
}
