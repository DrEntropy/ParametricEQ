/*
  ==============================================================================

    BypassButtonContainer.h
    Created: 9 May 2022 11:44:53am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DualBypassButton.h"

//==============================================================================
/*
*/
class BypassButtonContainer  : public juce::Component
{
public:
    BypassButtonContainer();
    ~BypassButtonContainer() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::array<std::unique_ptr<DualBypassButton>, 8> bypassButtons;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassButtonContainer)
};
