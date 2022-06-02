/*
  ==============================================================================

    ParameterAttachment.cpp
    Created: 2 Jun 2022 10:55:58am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "ParameterAttachment.h"

constexpr int callbackFreq = 60;

ParameterAttachment::ParameterAttachment (juce::RangedAudioParameter& param,
                                          std::function<void (float)> parameterChangedCallback,
                                          juce::UndoManager* um)
    : parameter (param),
      undoManager (um),
      setValue (std::move (parameterChangedCallback))
{
    parameter.addListener (this);
    startTimerHz(callbackFreq);
}

ParameterAttachment::~ParameterAttachment()
{
    stopTimer();
    parameter.removeListener (this);
}

void ParameterAttachment::sendInitialUpdate()
{
    parameterValueChanged ({}, parameter.getValue());
}

void ParameterAttachment::setValueAsCompleteGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
    {
        beginGesture();
        parameter.setValueNotifyingHost (f);
        endGesture();
    });
}

void ParameterAttachment::beginGesture()
{
    if (undoManager != nullptr)
        undoManager->beginNewTransaction();

    parameter.beginChangeGesture();
}

void ParameterAttachment::setValueAsPartOfGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
    {
        parameter.setValueNotifyingHost (f);
    });
}

void ParameterAttachment::endGesture()
{
    parameter.endChangeGesture();
}

void ParameterAttachment::parameterGestureChanged (int, bool)
{
    
}
 
void ParameterAttachment::parameterValueChanged (int, float newValue)
{
    lastValue = newValue;

    if (juce::MessageManager::getInstance()->isThisTheMessageThread() && setValue)
    {
        setValue (parameter.convertFrom0to1 (newValue));
    }
    else
    {
        lastValue.store(newValue);
        parameterChanged.set(true);
    }
}

float ParameterAttachment::normalise(float f) const
{
   return parameter.convertTo0to1(f);
}

void ParameterAttachment::timerCallback()
{
    if (parameterChanged.compareAndSetBool(false, true))
       if(setValue)
           setValue (parameter.convertFrom0to1 (lastValue));
}
