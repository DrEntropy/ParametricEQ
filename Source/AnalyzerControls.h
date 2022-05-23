/*
  ==============================================================================

    AnalyzerControls.h
    Created: 21 May 2022 4:00:03pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BottomLookAndFeel.h"
#include "BottomControl.h"
 
struct RotarySlider : juce::Slider
{
    RotarySlider() : juce::Slider(juce::Slider::SliderStyle::Rotary, juce::Slider::TextBoxBelow){}

};
//==============================================================================
/*
*/
class AnalyzerControls  : public juce::Component
{
public:
    AnalyzerControls(juce::AudioProcessorValueTreeState&);
    ~AnalyzerControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    
    BottomLookAndFeel lookAndFeel;
    
    juce::TextButton analyzerEnable{"On"};
    juce::Label buttonLabel{"AnalyzerLabel", "Analyzer"};
    
   // juce::Slider prePostSlider{juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextBoxRight};
    BottomControl<SwitchSlider> prePostSlider;
    BottomControl<SwitchSlider> pointsSlider;
    BottomControl<RotarySlider> decaySlider;
 
    std::unique_ptr<ButtonAttachment> analyzerEnableAttachment;
    
    
    
    void toggleEnablement();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyzerControls)
};
