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
AnalyzerControls::AnalyzerControls(juce::AudioProcessorValueTreeState& apvts) :
    prePostSlider(apvts, AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerProcessingMode)),
    pointsSlider(apvts, AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerPoints)),
    decaySlider(apvts, AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::AnalyzerDecayRate))
{
 
     
 
    analyzerEnableAttachment.reset(new ButtonAttachment(apvts,
                                                        AnalyzerProperties::getAnalyzerParamName(AnalyzerProperties::ParamNames::EnableAnalyzer),
                                                        analyzerEnable));;
    
    setLookAndFeel(&lookAndFeel);
    
    analyzerEnable.setClickingTogglesState(true);
    analyzerEnable.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    
    analyzerEnable.setToggleState(true, juce::NotificationType::dontSendNotification);
    
    auto safePtr = juce::Component::SafePointer<AnalyzerControls>(this);
   
    analyzerEnable.onClick = [safePtr]()
    {
     if(auto* comp = safePtr.getComponent() )
         comp->toggleEnablement();
    };
    
    buttonLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(buttonLabel);
    addAndMakeVisible(analyzerEnable);
    addAndMakeVisible(prePostSlider);
    addAndMakeVisible(pointsSlider);
    addAndMakeVisible(decaySlider);
}

AnalyzerControls::~AnalyzerControls()
{
    setLookAndFeel(nullptr);
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
    
    auto enableSection = bounds.removeFromLeft(width);
    
    analyzerEnable.setBounds(enableSection.removeFromBottom(width / 2).reduced(width / 10));
    buttonLabel.setBounds(enableSection);
    
    prePostSlider.setBounds(bounds.removeFromLeft(width));
    pointsSlider.setBounds(bounds.removeFromLeft(width));
    decaySlider.setBounds(bounds.removeFromRight(width));
}

void AnalyzerControls::toggleEnablement()
{
    bool state = analyzerEnable.getToggleState();
    prePostSlider.setEnabled(state);
    pointsSlider.setEnabled(state);
    decaySlider.setEnabled(state);
    if(state)
        analyzerEnable.setButtonText("On");
    else
        analyzerEnable.setButtonText("Off");
}
