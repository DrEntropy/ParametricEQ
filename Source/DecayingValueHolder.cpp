/*
  ==============================================================================

    DecayingValueHolder.cpp
    Created: 29 Apr 2022 10:28:16am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "DecayingValueHolder.h"


void DecayingValueHolder::updateHeldValue(float input)
{
    if(input > currentValue)
    {
        peakTime = getNow();
        currentValue = input;
        resetDecayRateMultiplier();
    }
        
}



float DecayingValueHolder::getCurrentValue() const
{
    return currentValue;
}

bool DecayingValueHolder::isOverThreshold() const
{
    return currentValue > threshold;
}

void DecayingValueHolder::setHoldTime(int ms)
{
    holdTime = ms;
}

void DecayingValueHolder::setDecayRate(float dbPerSec)
{
    decayRatePerFrame = dbPerSec / frameRate;
}

void DecayingValueHolder::timerCallback()
{
    auto now = getNow();
    if (now - peakTime > holdTime)
    {
        currentValue -= decayRatePerFrame * decayRateMultiplier;
        currentValue = juce::jlimit(NEGATIVE_INFINITY, MAX_DECIBELS, currentValue);
        decayRateMultiplier *= rateAccel;
        if(currentValue == NEGATIVE_INFINITY)
            resetDecayRateMultiplier();
    }
}

juce::int64 DecayingValueHolder::getNow()
{
    return juce::Time::currentTimeMillis();
}

void DecayingValueHolder::resetDecayRateMultiplier()
{
    decayRateMultiplier = 1;
}
