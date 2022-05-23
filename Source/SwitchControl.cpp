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
    auto myParameter = dynamic_cast<juce::AudioParameterChoice*>( apvts.getParameter(pName) );
    
    jassert(myParameter);  //switch control should only be used with choice parameters
    
    choices =  myParameter->choices;
    
    addAndMakeVisible(slider);
    label.setText(myParameter->name, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(label);
}

SwitchControl::~SwitchControl()
{
    attachment.reset();
}

void SwitchControl::paint (juce::Graphics& g)
{
    auto enabled = slider.isEnabled();
    auto startX = sliderBounds.getRight() + textHeight;
    auto startY = sliderBounds.getBottom() -sliderBounds.getWidth() / 2;
    auto width = getLocalBounds().getRight() - startX;
    
    auto deltaY =  sliderBounds.getHeight() - sliderBounds.getWidth();
    
    if(choices.size() > 1)
        deltaY /= (choices.size() - 1);
    
    g.setColour(enabled ? juce::Colours::white : juce::Colours::grey);
    
    for(auto choice : choices)
    {
       g.drawFittedText(choice, startX, startY - textHeight / 2, width, textHeight, juce::Justification::centredLeft , 1);
       startY -= deltaY;
    }
}

void SwitchControl::resized()
{
    auto bounds = getLocalBounds();
    auto w = bounds.getWidth();
    bounds.removeFromLeft(w / 6);
    label.setBounds(bounds.removeFromTop(w / 4));
    bounds.removeFromBottom(w / 6);
    sliderBounds = bounds.removeFromLeft(w / 9);
    slider.setBounds(sliderBounds);
}
