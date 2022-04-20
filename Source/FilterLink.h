/*
  ==============================================================================

    FilterLink.h
    Created: 19 Apr 2022 11:04:18am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include "FilterCoefficientGenerator.h"
#include "ReleasePool.h"
#include "Fifo.h"
#include "Decibel.h"
#include "CoeffTypeHelpers.h"





// FifoDataType is for example ReferenceCountedObjectPtr
// ParamType is one of the FilterParameters types.
// Function type is going to be CoefficientsMaker for now.
// FilterType is juce::dsp::IIR::Filter<float> or a chain of them.

template <typename FilterType, typename FifoDataType,  typename ParamType, typename FunctionType>
struct FilterLink
{
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        filter.prepare(spec);
        
        // update sampleRate
        initialize(ParamType{}, 0.0, true, spec.sampleRate);
        
    }
    
    void reset()
    {
        filter.reset();
    }
    
    template<typename ContextType>
    void process(const ContextType& context)
    {
        if(! currentParams.bypassed)
           filter.process(context);
    }
    
    //stuff for the juce::SmoothedValue instances.
    // commented out for now, will come back to when i get the rest working.
//    void updateSmootherTargets();
//    void resetSmoothers(float rampTime);
//    bool isSmoothing() const;
//    void checkIfStillSmoothing();
//    void advanceSmoothers(int numSamples);
    
    //stuff for updating the params
    void updateParams(const ParamType& params)
    {
       if(params != currentParams)
       {
           //coeffGen.changeParameters(params);
           shouldComputeNewCoefficients = true;
           currentParams = params;
       }
    }

    
    //stuff for updating the coefficients from processBlock, prepareToPlay, or setStateInformation
    void updateCoefficients(const FifoDataType& coefficents)
    {
        if constexpr( isReferenceCountedObjectPtr<FifoDataType>::value )
        {
            updateFilterState(filter.coefficients, coefficents);
        }
        else if constexpr ( isReferenceCountedArray<FifoDataType>::value )
        {
            configureCutFilterChain(coefficents);
        }
        else
        {
            jassertfalse;  // this should not happen
        }
    }
    
    
    void loadCoefficients(bool fromFifo)
    {
        if(fromFifo)
        {
            FifoDataType newCoefficients;
            while(coeffFifo.pull(newCoefficients))
            {
                updateCoefficients(newCoefficients);
            }
        }
        else
        {
            updateCoefficients(FunctionType::makeCoefficients(currentParams));
        }
    }
    void generateNewCoefficientsIfNeeded()
    {
        if(shouldComputeNewCoefficients.compareAndSetBool(false,true))
        {
            ParamType newParams {currentParams};
            // TODO update newParams using smoothed values for freq/gain/quality..
            // this will probably require some more type checking to see if it has a gain parameter or not.
            coeffGen.add(newParams);
            
        }
    }
    
    //stuff for configuring the filter before processing
    void performPreloopUpdate(const ParamType& params)
    {
        updateParams(params);
        
    }
    void performInnerLoopFilterUpdate(bool onRealTimeThread, int numSamplesToSkip);
    
    void initialize(const ParamType& params, float rampTime, bool onRealTimeThread, double sr)
    {
        currentParams = params;
        sampleRate = sr;
        shouldComputeNewCoefficients = true;
        genearteNewCoefficientsIfNeeded();
        loadCoefficients(onRealTimeThread);
        
    }
private:
    //stuff for setting the coefficients of the FilterType instance.

    void updateFilterState(FifoDataType& oldState, FifoDataType newState)
    {
        *oldState = *newState;
        releasePool.add(newState);
    }
    
    void configureCutFilterChain(const FifoDataType& coefficients)
    {
        // assumes CutFilter has 4 filters.
        filter.template setBypassed<0>(true);
        filter.template setBypassed<1>(true);
        filter.template setBypassed<2>(true);
        filter.template setBypassed<3>(true);
        
        int order {coefficients.size()};
        switch(order)
        {
            case 4:
                updateFilterState(filter.template get<3>().coefficients, coefficients[3]);
                filter.template setBypassed<3>(false);
            case 3:
                updateFilterState(filter.template get<2>().coefficients, coefficients[2]);
                filter.template setBypassed<2>(false);
            case 2:
                updateFilterState(filter.template get<1>().coefficients, coefficients[1]);
                filter.template setBypassed<1>(false);
            case 1:
                updateFilterState(filter.template get<0>().coefficients, coefficients[0]);
                filter.template setBypassed<0>(false);
        }
    }
    
    // magic numebrs
    static const int fifoSize = 100;
    static const int poolSize = 1000;
    static const int cleanupInterval = 2000; // ms

    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    
    float sampleRate;
    
    ParamType currentParams;
    
    juce::Atomic <bool> shouldComputeNewCoefficients{true};
 
    ReleasePool<Coefficients, poolSize> releasePool {poolSize, cleanupInterval};
    Fifo <FifoDataType, fifoSize>  coeffFifo;
    FilterCoefficientGenerator<FifoDataType, ParamType, CoefficientsMaker, fifoSize> coeffGen {coeffFifo};
    FilterType filter;
    
    // Smoothers
//    juce::SmoothedValue<float> freqSmoother;
//    juce::SmoothedValue<float> qualitySmoother;
//    juce::SmoothedValue<Decibel<float>> gainSmoother;
    
};
