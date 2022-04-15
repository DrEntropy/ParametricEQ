/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HighCutLowCutParameters.h"
#include "FilterParameters.h"
#include "Fifo.h"
#include "CoefficientsMaker.h"
#include "ParameterHelpers.h"
#include "FilterCoefficientGenerator.h"


using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter,Filter,CutFilter>;


 


//==============================================================================
/**
*/
class ParametricEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ParametricEQAudioProcessor();
    ~ParametricEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // =========================================================================
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Params", createParameterLayout() };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
    
    template <const int filterNum>
    void updateParametricFilter(double sampleRate, bool forceUpdate)
    {
        using namespace FilterInfo;


        
        float frequency = apvts.getRawParameterValue(createFreqParamString(filterNum))->load();
        float quality  = apvts.getRawParameterValue(createQParamString(filterNum))->load();
        bool bypassed = apvts.getRawParameterValue(createBypassParamString(filterNum))->load() > 0.5f;
        
        FilterType filterType = static_cast<FilterType> (apvts.getRawParameterValue(createTypeParamString(filterNum))->load());
        
        if (filterType == FilterType::LowPass || filterType == FilterType::HighPass || filterType == FilterType::FirstOrderHighPass || filterType == FilterType::FirstOrderLowPass)
        {
            HighCutLowCutParameters cutParams;
            
            cutParams.isLowcut = (filterType == FilterType::HighPass) || (filterType == FilterType::FirstOrderHighPass);
            cutParams.frequency = frequency;
            cutParams.bypassed = bypassed;
            cutParams.order = 1;
            
            if (filterType == FilterType::HighPass || filterType == FilterType::LowPass)
                cutParams.order = 2;
                
            cutParams.sampleRate = sampleRate;
            cutParams.quality  = quality;
            
            
            if (forceUpdate || filterType != oldFilterType || cutParams != oldCutParams)
            {
                cutCoeffGen.changeParameters(cutParams);
            }
            
            CutCoeffArray newChainCoefficients;
            bool newChainAvailable = cutCoeffFifo.pull(newChainCoefficients);
            
            
            if (newChainAvailable)
            {
                leftChain.setBypassed<filterNum>(bypassed);
                rightChain.setBypassed<filterNum>(bypassed);
                
                *(leftChain.get<filterNum>().coefficients) = *(newChainCoefficients[0]);
                *(rightChain.get<filterNum>().coefficients) = *(newChainCoefficients[0]);
            }
        
            oldCutParams = cutParams;
        }
        else
        {
            FilterParameters parametricParams;
            
            parametricParams.frequency = frequency;
            parametricParams.filterType = filterType;
            parametricParams.sampleRate = sampleRate;
            parametricParams.quality = quality;
            parametricParams.bypassed = bypassed;
            parametricParams.gain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue(createGainParamString(filterNum))-> load());
            
            // set up filter chains.
            if (forceUpdate || filterType != oldFilterType || parametricParams != oldParametricParams)
            {
                parametricCoeffGen.changeParameters(parametricParams);
            }
            
            ParametricCoeffPtr newChainCoefficients;
            bool newChainAvailable = parametricCoeffFifo.pull(newChainCoefficients);

            if (newChainAvailable)
            {
                leftChain.setBypassed<filterNum>(bypassed);
                rightChain.setBypassed<filterNum>(bypassed);
                *(leftChain.get<filterNum>().coefficients) = *newChainCoefficients;
                *(rightChain.get<filterNum>().coefficients) = *newChainCoefficients;
                
                // placeholder until release pool will take care of this Remember to get rid of this
                newChainCoefficients.get()->incReferenceCount();
            }
            
            oldParametricParams = parametricParams;
            oldFilterType = filterType;
        }
    }
    
    template <const int filterNum>
    void updateCutFilter(double sampleRate, bool forceUpdate, HighCutLowCutParameters& oldParams, bool isLowCut)
    {
        using namespace FilterInfo;
        
        float frequency = apvts.getRawParameterValue(createFreqParamString(filterNum))->load();
        bool bypassed = apvts.getRawParameterValue(createBypassParamString(filterNum))->load() > 0.5f;
        
        Slope slope = static_cast<Slope> (apvts.getRawParameterValue(createSlopeParamString(filterNum))->load());
        
        HighCutLowCutParameters cutParams;
            
        cutParams.isLowcut = isLowCut;
        cutParams.frequency = frequency;
        cutParams.bypassed = bypassed;
        cutParams.order = static_cast<int>(slope) + 1;
        cutParams.sampleRate = sampleRate;
        cutParams.quality  = 1.0f; //not used for cut filters
        
       
        //  send parameters if they changed
        if (forceUpdate || oldParams != cutParams)
        {
            if(isLowCut)
            {
                lowCutCoeffGen.changeParameters(cutParams);
    
            }
            else
            {
                highCutCoeffGen.changeParameters(cutParams);
            }
            
        }
        
           
        CutCoeffArray newChainCoefficients;
        bool newChainAvailable;
        
        if(isLowCut)
        {
            newChainAvailable = lowCutCoeffFifo.pull(newChainCoefficients);
        }
        else
        {
            newChainAvailable = highCutCoeffFifo.pull(newChainCoefficients);
        }
        
        if (newChainAvailable)
        {
            leftChain.setBypassed<filterNum>(bypassed);
            rightChain.setBypassed<filterNum>(bypassed);
            bypassSubChain<filterNum>();
            //set up the four filters
            if(!bypassed)
            {
                int order {newChainCoefficients.size()};
                switch(order)
                {
                    case 4:
                        updateSingleCut<filterNum,3> (newChainCoefficients);
                    case 3:
                        updateSingleCut<filterNum,2> (newChainCoefficients);
                    case 2:
                        updateSingleCut<filterNum,1> (newChainCoefficients);
                    case 1:
                        updateSingleCut<filterNum,0> (newChainCoefficients);
                   }
            }
        }
        
        // side effect update. Code smell?
        oldParams = cutParams;
    }
    
    template <const int filterNum, const int subFilterNum, typename CoefficientType>
    void updateSingleCut(CoefficientType& chainCoefficients)
    {
        auto& leftSubChain = leftChain.template get<filterNum>();
        auto& rightSubChain = rightChain.template get<filterNum>();
        
        
        
        *(leftSubChain.template get<subFilterNum>().coefficients) = *(chainCoefficients[subFilterNum]);
        *(rightSubChain.template get<subFilterNum>().coefficients) = *(chainCoefficients[subFilterNum]);
        
        leftSubChain.template setBypassed<subFilterNum>(false);
        rightSubChain.template setBypassed<subFilterNum>(false);
        
    }
    
    template <const int filterNum>
    void bypassSubChain()
    {
        auto& leftSubChain = leftChain.template get<filterNum>();
        auto& rightSubChain = rightChain.template get<filterNum>();
        leftSubChain.template setBypassed<0>(true);
        leftSubChain.template setBypassed<1>(true);
        leftSubChain.template setBypassed<2>(true);
        leftSubChain.template setBypassed<3>(true);
        rightSubChain.template setBypassed<0>(true);
        rightSubChain.template setBypassed<1>(true);
        rightSubChain.template setBypassed<2>(true);
        rightSubChain.template setBypassed<3>(true);
    }
  
    void updateFilters(double sampleRate, bool forceUpdate = false);
    
    using ParamLayout = juce::AudioProcessorValueTreeState::ParameterLayout;
    
    void addFilterParamToLayout(ParamLayout&, int,bool);
 
    ParamLayout createParameterLayout();
    MonoChain leftChain, rightChain;
    
    HighCutLowCutParameters oldCutParams;
    FilterParameters oldParametricParams;
    FilterInfo::FilterType oldFilterType;
    
    HighCutLowCutParameters oldHighCutParams;
    HighCutLowCutParameters oldLowCutParams;
    
    //using ParametricCoeffPtr = decltype(CoefficientsMaker::makeCoefficients (oldParametricParams));
    //using CutCoeffArray = decltype(CoefficientsMaker::makeCoefficients (oldLowCutParams));

    Fifo <ParametricCoeffPtr,100>  parametricCoeffFifo;
    Fifo <CutCoeffArray,100>  cutCoeffFifo;
    
    Fifo <CutCoeffArray,100>  lowCutCoeffFifo;
    Fifo <CutCoeffArray,100>  highCutCoeffFifo;
    
    // FOUR filter coefficient generators, due to the special case of the central cut filter.
    // Will probably delete this central cut filter, because when we have 8 of these it will be a nightmare
    // and also have no value since we get teh same filter for order 1 and 2 from the makeLowPass, etc
    
    FilterCoefficientGenerator<CutCoeffArray, HighCutLowCutParameters, CoefficientsMaker, 100> highCutCoeffGen {highCutCoeffFifo};
    FilterCoefficientGenerator<CutCoeffArray, HighCutLowCutParameters, CoefficientsMaker, 100> lowCutCoeffGen {lowCutCoeffFifo};
    FilterCoefficientGenerator<CutCoeffArray, HighCutLowCutParameters, CoefficientsMaker, 100> cutCoeffGen {cutCoeffFifo};
    FilterCoefficientGenerator<ParametricCoeffPtr, FilterParameters, CoefficientsMaker, 100> parametricCoeffGen {parametricCoeffFifo};
};
