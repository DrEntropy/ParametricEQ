/*
  ==============================================================================

    SwitchControl.h
    Created: 22 May 2022 3:51:55pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BottomLookAndFeel.h"

//==============================================================================
/*
*/
class SwitchControl  : public juce::Component
{
public:
    SwitchControl(juce::AudioProcessorValueTreeState&, juce::String);
    ~SwitchControl() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    juce::AudioProcessorValueTreeState& apvts;
    juce::String parameterName;
    juce::Slider slider{juce::Slider::SliderStyle::LinearVertical, juce::Slider::NoTextBox};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    juce::Label label;
    
    juce::Rectangle<int> sliderBounds;
    juce::StringArray choices;
    
    static constexpr int textHeight = 14;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwitchControl)
};
