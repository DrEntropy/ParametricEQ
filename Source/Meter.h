/*
  ==============================================================================

    Meter.h
    Created: 27 Apr 2022 9:53:41am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DecayingValueHolder.h"
#include "EQConstants.h"
#include "Averager.h"

#define TICK_INTERVAL 6
#define DECAY_BAR_THICK 4.f
#define INNER_TICK_INTERVAL 12
#define INNER_TICK_THICK 2.f

//==============================================================================
/*
*/


class Meter  : public juce::Component
{
public:
    Meter(juce::String);
    ~Meter() override;

    void paint (juce::Graphics&) override;
    
    void update(float dbLevelPeak, float dbLevelRMS);
    
    juce::Rectangle<int> getMeterBounds();
    
private:
    
    void paintBar (juce::Graphics& g, float value, juce::Rectangle<float> bounds, float dWidth, juce::Colour color);
    
    void paintInnerTicks(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color);
    
    float peakDb { NEGATIVE_INFINITY };
    DecayingValueHolder decayingValueHolder;
    Averager<float> averageDb { static_cast<size_t>(FRAME_RATE * AVG_TIME), NEGATIVE_INFINITY };
    
    const juce::String chanLabel;
    
    static const int labelSize {30};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Meter)
};
