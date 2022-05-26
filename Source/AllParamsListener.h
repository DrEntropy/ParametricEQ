/*
  ==============================================================================

    AllParamsListener.h
    Created: 26 May 2022 11:52:53am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct AllParamsListener : juce::Timer, juce::AudioProcessorParameter::Listener
{
    AllParamsListener(juce::AudioProcessorValueTreeState& apv,
                      std::function<void()> f);
    ~AllParamsListener() override;
    void timerCallback() override;
    void parameterValueChanged (int /*parameterIndex*/, float /*newValue*/) override;
    void parameterGestureChanged (int /*parameterIndex*/, bool /*gestureIsStarting*/) override;
private:
    juce::AudioProcessorValueTreeState* apvts;
    std::function<void()> func;
    juce::Atomic<bool> changed { false };
};
