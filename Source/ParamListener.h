/*
  ==============================================================================

    ParamListener.h
    Created: 4 May 2022 9:26:20am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define POLLING_FREQ 60

struct ParamListener : juce::Timer
{
    ParamListener(juce::RangedAudioParameter* parameter, std::function<void (float)> lambda);

    void timerCallback() override;
    
private:
    juce::RangedAudioParameter*  audioParameter;
    float cachedValue;
    std::function<void (float)> listener;
};
