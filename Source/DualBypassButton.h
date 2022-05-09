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
    DualBypassButton(int filterNum);
    ~DualBypassButton() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int filterNum;
    BypassButton leftMidBypass;
    BypassButton rightSideBypass;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualBypassButton)
};
