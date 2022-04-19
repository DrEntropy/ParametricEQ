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

// FifoDataType is for example ReferenceCountedObjectPtr
// ParamType is one of the FilterParameters types.
// Function type is going to be CoefficientsMaker for now.
// FilterType is juce::dsp::IIR::Filter<float> or a chain of them.

template <typename FilterType, FifoDataType, ParamType, FunctionType>
struct FilterLink
{
    void prepare(const dsp::ProcessSpec&);
    void reset();
    
    template<typename ContextType>
    void process(const ContextType& context);
    
    //stuff for the juce::SmoothedValue instances.
    void updateSmootherTargets();
    void resetSmoothers(float rampTime);
    bool isSmoothing() const;
    void checkIfStillSmoothing();
    void advanceSmoothers(int numSamples);
    
    //stuff for updating the params
    void updateParams(const ParamType& params);
    
    //stuff for updating the coefficients from processBlock, prepareToPlay, or setStateInformation
    void updateCoefficients(const FifoDataType& coefficents);
    void loadCoefficients(bool fromFifo);
    void generateNewCoefficientsIfNeeded();
    
    //stuff for configuring the filter before processing
    void performPreloopUpdate(const ParamType& params);
    void performInnerLoopFilterUpdate(bool onRealTimeThread, int numSamplesToSkip);
    void initialize(const ParamType& params, float rampTime, bool onRealTimeThread, double sr);
private:
    //stuff for setting the coefficients of the FilterType instance.
    updateFilterState(Ptr& oldState, Ptr newState);
    void configureCutFilterChain(const FifoDataType& coefficients);
    
    // magic numebrs
    static const int fifoSize = 100;
    static const int poolSize = 1000;
    static const int cleanupInterval = 2000; // ms

    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    
    float sampleRate;
    
    ParamType currentParams;
    
    bool shouldComputeNewCoefficients;
 
    ReleasePool<Coefficients, poolSize> releasePool {poolSize, cleanupInterval};
    Fifo <FifoDataType, fifoSize>  coeffFifo;
    FilterCoefficientGenerator<FifoDataType, ParamType, CoefficientsMaker, fifoSize> coeffGen {coeffFifo};
    FilterType myFilter;
    
    
};
