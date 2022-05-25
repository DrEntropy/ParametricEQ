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

void AnalyzerControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(bounds.toFloat());
}

void AnalyzerControls::resized()
{
    auto bounds = getLocalBounds();
    auto width = bounds.getHeight(); // use square sections
    
    auto enableSection = bounds.removeFromLeft(width);
    
    analyzerEnable.setBounds(enableSection.removeFromBottom(width / 2).reduced(width / marginDiv));
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
    analyzerEnable.setButtonText(state ? "On" : "Off");
}
