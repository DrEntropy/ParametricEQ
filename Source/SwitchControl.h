/*
  ==============================================================================

    SwitchControl.h
    Created: 22 May 2022 3:51:55pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
 
#include "SwitchSlider.h"

//==============================================================================
/*
*/
class SwitchControl  : public juce::Component
{
public:
    SwitchControl(juce::AudioProcessorValueTreeState&, juce::String);
    ~SwitchControl() override;

    void resized() override;

private:
    
    SwitchSlider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    juce::Label label;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwitchControl)
};
