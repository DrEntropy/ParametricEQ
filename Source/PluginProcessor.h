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
            
            // set up filter chains.
           
            if (forceUpdate || filterType != oldFilterType || cutParams != oldCutParams)
            {
                auto chainCoefficients = CoefficientsMaker::makeCoefficients(cutParams);
                
                cutCoeffFifo.push(chainCoefficients);
                decltype(chainCoefficients) newChainCoefficients;
                cutCoeffFifo.pull(newChainCoefficients);
                
                leftChain.setBypassed<filterNum>(bypassed);
                rightChain.setBypassed<filterNum>(bypassed);
                
                // Later this will be multiple filters for each of the bands i think.
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
                auto chainCoefficients = CoefficientsMaker::makeCoefficients(parametricParams);
                
                // push and pull Fifo for testing.
                bool temp0 = parametricCoeffFifo.push(chainCoefficients);
                jassert(temp0);
                
                // pull a copy
                ParametricCoeffPtr newChainCoefficients;
                bool temp = parametricCoeffFifo.pull(newChainCoefficients);
                jassert(temp);
                

                
                leftChain.setBypassed<filterNum>(bypassed);
                rightChain.setBypassed<filterNum>(bypassed);
                *(leftChain.get<filterNum>().coefficients) = *newChainCoefficients;
                *(rightChain.get<filterNum>().coefficients) = *newChainCoefficients;
                
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
        
       
        
        if (forceUpdate || oldParams != cutParams)
        {
            auto chainCoefficients = CoefficientsMaker::makeCoefficients(cutParams);
            decltype(chainCoefficients) newChainCoefficients;
            
            if(isLowCut)
            {
                lowCutCoeffFifo.push(chainCoefficients);
                lowCutCoeffFifo.pull(newChainCoefficients);
            }
            else
            {
                highCutCoeffFifo.push(chainCoefficients);
                highCutCoeffFifo.pull(newChainCoefficients);
            }
        
            leftChain.setBypassed<filterNum>(bypassed);
            rightChain.setBypassed<filterNum>(bypassed);
            bypassSubChain<filterNum>();
            //set up the four filters
            if(!bypassed)
            {
                
                switch(slope)
                {
                    case Slope::Slope_48:
                    case Slope::Slope_42:
                        updateSingleCut<filterNum,3> (newChainCoefficients);
                        
                    case Slope::Slope_36:
                    case Slope::Slope_30:
                        updateSingleCut<filterNum,2> (newChainCoefficients);
                        
                    case Slope::Slope_24:
                    case Slope::Slope_18:
                        updateSingleCut<filterNum,1> (newChainCoefficients);
               
                    case Slope::Slope_12:
                    case Slope::Slope_6:
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
    
    using ParametricCoeffPtr = decltype(CoefficientsMaker::makeCoefficients (oldParametricParams));
    using CutCoeffArray = decltype(CoefficientsMaker::makeCoefficients (oldLowCutParams));

    Fifo <ParametricCoeffPtr,10>  parametricCoeffFifo;
    Fifo <CutCoeffArray,10>  cutCoeffFifo;
    
    Fifo <CutCoeffArray,10>  lowCutCoeffFifo;
    Fifo <CutCoeffArray,10>  highCutCoeffFifo;
};
