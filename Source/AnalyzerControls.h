/*
  ==============================================================================

    AnalyzerControls.h
    Created: 21 May 2022 4:00:03pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
 

//==============================================================================
/*
*/
class AnalyzerControls  : public juce::Component
{
public:
    AnalyzerControls(juce::AudioProcessorValueTreeState&);
    ~AnalyzerControls() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    juce::ToggleButton analyzerEnable;
    juce::Slider prePostSlider{juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextBoxRight};
    juce::Slider pointsSlider{juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextBoxRight};
    juce::Slider decaySlider{juce::Slider::SliderStyle::Rotary, juce::Slider::TextBoxAbove};
    
    std::unique_ptr<SliderAttachment> prePostAttachment;
    std::unique_ptr<SliderAttachment> pointsAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<ButtonAttachment> analyzerEnableAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyzerControls)
};
