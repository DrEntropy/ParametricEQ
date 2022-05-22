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
SwitchControl::SwitchControl(juce::AudioProcessorValueTreeState& apvtsIn, juce::String pName) : apvts {apvtsIn}, parameterName {pName}
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, parameterName, slider));
    addAndMakeVisible(slider);
}

SwitchControl::~SwitchControl()
{
    attachment.reset();
}

void SwitchControl::paint (juce::Graphics& g)
{
     // labels to switch positions.
}

void SwitchControl::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    slider.setBounds(getLocalBounds());
}
