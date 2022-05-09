/*
  ==============================================================================

    DualBypassButton.h
    Created: 9 May 2022 11:44:33am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BypassButton.h"

//==============================================================================
/*
*/
class DualBypassButton  : public juce::Component
{
public:
    DualBypassButton(int filterNum, juce::AudioProcessorValueTreeState& apvts);
    ~DualBypassButton() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int filterNum;
    BypassButton leftMidBypass;
    BypassButton rightSideBypass;
    
    //juce::ToggleButton leftMidBypass;
    //juce::ToggleButton rightSideBypass;
    
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> leftMidAttachment;
    std::unique_ptr<ButtonAttachment> rightSideAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualBypassButton)
};
