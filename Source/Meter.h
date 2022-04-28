/*
  ==============================================================================

    Meter.h
    Created: 27 Apr 2022 9:53:41am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define NEGATIVE_INFINITY -66.0f
#define MAX_DECIBELS 12.0f

//==============================================================================
/*
*/


class Meter  : public juce::Component
{
public:
    Meter();
    ~Meter() override;

    void paint (juce::Graphics&) override;
    
    void update(float dbLevel);

private:
    float peakDb { NEGATIVE_INFINITY };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Meter)
};
