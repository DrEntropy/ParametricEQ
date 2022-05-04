/*
  ==============================================================================

    ParamListener.cpp
    Created: 4 May 2022 9:34:45am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "ParamListener.h"


ParamListener::ParamListener(juce::RangedAudioParameter* parameter, std::function<void(float)> lambda) : audioParameter (parameter), listener (lambda)
{
    jassert(parameter);
    jassert(lambda);
    
    cachedValue = audioParameter->getValue();
    
    startTimerHz(POLLING_FREQ);
}

void ParamListener::timerCallback()
{
    float value = audioParameter->getValue();
    
    if(value != cachedValue)
    {
        cachedValue = value;
        listener(audioParameter->convertFrom0to1(value));
    }
}
