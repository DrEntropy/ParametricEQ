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

//==============================================================================
/*
*/
class StereoMeter  : public juce::Component
{
public:
    StereoMeter();
    ~StereoMeter() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void update(MeterValues values);

private:
    
    Meter leftMeter, rightMeter;
    DbScale dBScale;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoMeter)
};
