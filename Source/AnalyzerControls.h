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
 

//==============================================================================
/*
*/
class AnalyzerControls  : public juce::Component
{
public:
    AnalyzerControls(juce::AudioProcessorValueTreeState&);
    ~AnalyzerControls() override;
    
 
    void resized() override;

private:
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    
    BottomLookAndFeel lookAndFeel;
    
    juce::TextButton analyzerEnable{"On"};
    juce::Label buttonLabel{"AnalyzerLabel", "Analyzer"};
    
    BottomControl<SwitchSlider> prePostSlider;
    BottomControl<SwitchSlider> pointsSlider;
    BottomControl<RotarySlider> decaySlider;
    
    BoundaryBox boundaryBox;
 
    std::unique_ptr<ButtonAttachment> analyzerEnableAttachment;
    
    void toggleEnablement();
    
    static constexpr int marginDiv {10};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyzerControls)
};
