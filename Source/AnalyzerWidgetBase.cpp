/*
  ==============================================================================

    AnalyzerWidgetBase.cpp
    Created: 30 May 2022 10:08:36am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalyzerWidgetBase.h"

//==============================================================================
AnalyzerWidgetBase::AnalyzerWidgetBase()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

AnalyzerWidgetBase::~AnalyzerWidgetBase()
{
}

void AnalyzerWidgetBase::paint (juce::Graphics& g)
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
    g.drawText ("AnalyzerWidgetBase", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void AnalyzerWidgetBase::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
