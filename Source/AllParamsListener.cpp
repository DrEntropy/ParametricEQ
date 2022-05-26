/*
  ==============================================================================

    AllParamsListener.cpp
    Created: 26 May 2022 11:52:53am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "AllParamsListener.h"

AllParamsListener::AllParamsListener(juce::AudioProcessorValueTreeState& apv, std::function<void()> f) : apvts{apv}, func {f}
{
    auto params = apvts.processor.getParameters();
    for (auto param : params)
        param -> addListener(this);
    
    startTimerHz(60);
}

AllParamsListener::~AllParamsListener()
{
    stopTimer();
    auto params = apvts.processor.getParameters();
    for (auto param : params)
        param -> removeListener(this);
}

void AllParamsListener::timerCallback()
{
    if(changed.compareAndSetBool(false, true))
       func();
}

void AllParamsListener::parameterValueChanged (int paramIndex, float newValue)
{
    changed.set(true);
}

void AllParamsListener::parameterGestureChanged (int paramIndex, bool gestureStarting)
{
    //nothing
}

