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
#include "SwitchControl.h"
 

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
    SwitchControl prePostSlider;
    SwitchControl pointsSlider;
    juce::Slider decaySlider{juce::Slider::SliderStyle::Rotary, juce::Slider::TextBoxBelow};
    
    std::unique_ptr<SliderAttachment> prePostAttachment;
    std::unique_ptr<SliderAttachment> pointsAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<ButtonAttachment> analyzerEnableAttachment;
    
    
    
    void toggleEnablement();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyzerControls)
};
