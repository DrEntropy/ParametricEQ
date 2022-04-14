/*
  ==============================================================================

    FilterCoefficientGenerator.h
    Created: 14 Apr 2022 12:57:25pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fifo.h"
#include "CoefficientsMaker.h"




template <typename CoefficientType, typename ParamType, typename MakeFunction, size_t Size>
struct FilterCoefficientGenerator : juce::Thread
{
    FilterCoefficientGenerator(Fifo <CoefficientType, Size>& processFifo) : Thread {"Coefficient Maker Thread"}, coeffFifo {processFifo}
    {
        startThread();
    }
    
    ~FilterCoefficientGenerator() override
    {
        bool stopSucceeded =  stopThread(2000);
        jassert(stopSucceeded);
    }
    
    void changeParameters(ParamType params)
    {
        paramFifo.push(params);
        paramChanged.set(true);
    }
    
    void run() override
    {
        while(! threadShouldExit())
        {
            wait (waitTime);
            if (paramChanged.compareAndSetBool (false, true))
            {
                while (paramFifo.getNumAvailableForReading() >0)
                    {
                        ParamType params;
                        paramFifo.pull(params);
                        auto coeffs = MakeFunction::makeCoefficients(params);
                        coeffFifo.push(coeffs);
                    }
            }
            
        }
    }
private:
    int waitTime = 10;  //ms
    
    Fifo <CoefficientType, Size>& coeffFifo;
    Fifo <ParamType, 100> paramFifo;
    
    juce::Atomic<bool> paramChanged{false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterCoefficientGenerator)
};
