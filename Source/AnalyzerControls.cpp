/*
  ==============================================================================

    AnalyzerControls.cpp
    Created: 21 May 2022 4:00:03pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalyzerControls.h"
#include "AnalyzerProperties.h"

//==============================================================================
AnalyzerControls::AnalyzerControls(juce::AudioProcessorValueTreeState& apvts)
{
    testAttachment.reset(new SliderAttachment(apvts,
                             AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerProcessingMode),
                             testSlider));
    
    addAndMakeVisible(testSlider);
}


void AnalyzerControls::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    auto bounds = getLocalBounds();

    
    
    g.setColour (juce::Colours::red);
    g.drawRect (bounds, 1);   // draw an outline around the component
 
}

void AnalyzerControls::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    testSlider.setBounds(getLocalBounds().removeFromLeft(100));
}
