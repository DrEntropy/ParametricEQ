/*
  ==============================================================================

    FilterLink.h
    Created: 19 Apr 2022 11:04:18am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once


template<typename FilterType, FifoDataType, ParamType, FunctionType>
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
};
