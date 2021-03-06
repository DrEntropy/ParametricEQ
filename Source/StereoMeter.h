/*
  ==============================================================================

    StereoMeter.h
    Created: 2 May 2022 1:31:26pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MeterValues.h"
#include "Meter.h"
#include "DbScale.h"
#include "Fifo.h"

#define LABEL_SPACE 30
#define METER_WIDTH 25

//==============================================================================
/*
*/
class StereoMeter  : public juce::Component
{
public:
    StereoMeter(juce::String);
    ~StereoMeter() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void update(MeterValues values);

private:
    
    Meter leftMeter {"L"}, rightMeter {"R"};
    DbScale dBScale;
    
    const juce::String meterLabel;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoMeter)
};
