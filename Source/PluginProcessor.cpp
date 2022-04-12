/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "ParameterHelpers.h"
#include "FilterInfo.h"
#include "FilterParameters.h"
#include "HighCutLowCutParameters.h"

 
#include <string>

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ParametricEQAudioProcessor::~ParametricEQAudioProcessor()
{
}

//==============================================================================
const juce::String ParametricEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParametricEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParametricEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParametricEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParametricEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParametricEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParametricEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParametricEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ParametricEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters(sampleRate,true);
}

void ParametricEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ParametricEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    
    


    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
     updateFilters(getSampleRate());
     
     juce::dsp::AudioBlock<float> block(buffer);
     auto leftBlock = block.getSingleChannelBlock(0);
     auto rightBlock = block.getSingleChannelBlock(1);
     
    
     juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
     juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
     leftChain.process(leftContext);
     rightChain.process(rightContext);
}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
   // return new ParametricEQAudioProcessorEditor (*this);
    return new  juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ParametricEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}

void ParametricEQAudioProcessor::addFilterParamToLayout (ParamLayout& layout, int filterNum, bool isCut)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(createBypassParamString(filterNum),createBypassParamString(filterNum),false) );
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(createFreqParamString(filterNum), createFreqParamString(filterNum),
                                       juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20.0f));
    

    
    if(!isCut)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(createQParamString(filterNum), createQParamString(filterNum),
                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.0f), 1.0f));
        
        layout.add(std::make_unique<juce::AudioParameterFloat>(createGainParamString(filterNum),createGainParamString(filterNum),
                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.0f), 0.0f));
        juce::StringArray types;
        
        for (const auto& [type, stringRep] : FilterInfo::mapFilterTypeToString)
        {
            types.add(stringRep);
        }

        layout.add(std::make_unique<juce::AudioParameterChoice>(createTypeParamString(filterNum), createTypeParamString(filterNum), types, 0));
    }
    else
    {
        juce::StringArray slopes;
        
        for (const auto& [order, stringRep] : FilterInfo::mapSlopeToString)
        {
            slopes.add(stringRep);
        }

        layout.add(std::make_unique<juce::AudioParameterChoice>(createSlopeParamString(filterNum),
                                                                createSlopeParamString(filterNum), slopes, 0));
    }
}



juce::AudioProcessorValueTreeState::ParameterLayout ParametricEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    
    addFilterParamToLayout(layout, 0, true);
    addFilterParamToLayout(layout, 1, false);
    addFilterParamToLayout(layout, 2, true);
    
    return layout;
    

}

template <const int filterNum>
void ParametricEQAudioProcessor::updateParametricFilter(double sampleRate, bool forceUpdate)
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
            parametricCoeffFifo.push(chainCoefficients);
            decltype(chainCoefficients) newChainCoefficients;
            parametricCoeffFifo.pull(newChainCoefficients);
            
            leftChain.setBypassed<filterNum>(bypassed);
            rightChain.setBypassed<filterNum>(bypassed);
            *(leftChain.get<filterNum>().coefficients) = *newChainCoefficients;
            *(rightChain.get<filterNum>().coefficients) = *newChainCoefficients;
        }
        
        oldParametricParams = parametricParams;
    }
}

// to do - DRY
template<const int filterNum>
void ParametricEQAudioProcessor::updateCutFilter(double sampleRate, bool forceUpdate,
                                                 HighCutLowCutParameters& oldParams, bool isLowCut)
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


void ParametricEQAudioProcessor::updateFilters(double sampleRate, bool forceUpdate)
{
    updateCutFilter<0>(sampleRate, forceUpdate, oldHighCutParams, true);
    updateParametricFilter<1>(sampleRate, forceUpdate);
    updateCutFilter<2>(sampleRate, forceUpdate, oldLowCutParams, false);
    
}
