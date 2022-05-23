/*
  ==============================================================================

    KnobControl.h
    Created: 23 May 2022 12:20:34am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class KnobControl  : public juce::Component
{
public:
    KnobControl();
    ~KnobControl() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobControl)
};
