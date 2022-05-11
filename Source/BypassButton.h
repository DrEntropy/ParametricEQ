/*
  ==============================================================================

    BypassButton.h
    Created: 9 May 2022 11:43:57am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define SINGLE_BYPASS_MARGIN 5

class BypassButton : public juce::Button
{
public:
    BypassButton();
    ~BypassButton() override = default;
    
    virtual void paintButton (juce::Graphics& g,bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    bool onLeft{true}, isPaired{false};
    
    bool isShowingAsOn();
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassButton)
};

