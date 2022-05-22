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
    prePostAttachment.reset(new SliderAttachment(apvts,
                             AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerProcessingMode),
                             prePostSlider));
    
     
    pointsAttachment.reset(new SliderAttachment(apvts,
                                                AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerPoints),
                                                pointsSlider));;
    decayAttachment.reset(new SliderAttachment(apvts,
                                               AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerDecayRate),
                                               decaySlider));;
    analyzerEnableAttachment.reset(new ButtonAttachment(apvts,
                                                        AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::EnableAnalyzer),
                                                        analyzerEnable));;
    
    addAndMakeVisible(analyzerEnable);
    addAndMakeVisible(prePostSlider);
    addAndMakeVisible(pointsSlider);
    addAndMakeVisible(decaySlider);
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
    auto bounds = getLocalBounds();
    auto width = bounds.getHeight();
    analyzerEnable.setBounds(bounds.removeFromLeft(width));
    prePostSlider.setBounds(bounds.removeFromLeft(width));
    pointsSlider.setBounds(bounds.removeFromLeft(width));
    decaySlider.setBounds(bounds);
}
