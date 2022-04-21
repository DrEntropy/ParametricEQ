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
    
    
    initializeFilters(sampleRate);

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

    performPreLoopUpdate(getSampleRate());
    
    juce::dsp::AudioBlock<float> block(buffer);

    
    int numSamples = buffer.getNumSamples();
    int offset = 0;
    
    while(offset < numSamples)
    {
        int blockSize = std::min(numSamples - offset, innerLoopSize);
        auto subBlock =  block.getSubBlock(offset, blockSize);
        auto leftBlock = subBlock.getSingleChannelBlock(0);
        auto rightBlock = subBlock.getSingleChannelBlock(1);
        
        performInnerLoopUpdate(getSampleRate(), blockSize);
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        leftChain.process(leftContext);
        rightChain.process(rightContext);
        offset += innerLoopSize;
    }
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

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
     if (xmlState.get() != nullptr)
                if (xmlState->hasTagName (apvts.state.getType()))
                    apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
    initializeFilters(getSampleRate());
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

        layout.add(std::make_unique<juce::AudioParameterChoice>(createTypeParamString(filterNum), createTypeParamString(filterNum), types, 10));
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
    addFilterParamToLayout(layout, 2, false);
    addFilterParamToLayout(layout, 3, false);
    addFilterParamToLayout(layout, 4, false);
    addFilterParamToLayout(layout, 5, false);
    addFilterParamToLayout(layout, 6, false);
    addFilterParamToLayout(layout, 7, true);
    
    return layout;
}


void ParametricEQAudioProcessor::initializeFilters(double sampleRate)
{
    // check if on realtime thread
    auto messMan = juce::MessageManager::getInstanceWithoutCreating();
    bool onRealTimeThread=  ! ((messMan != nullptr) && messMan->isThisTheMessageThread());
    
    // initialize filters
   
    initializeChain<1>(getParametericFilterParams<1>(sampleRate), onRealTimeThread, sampleRate);
    initializeChain<2>(getParametericFilterParams<2>(sampleRate), onRealTimeThread, sampleRate);
    initializeChain<3>(getParametericFilterParams<3>(sampleRate), onRealTimeThread, sampleRate);
    initializeChain<4>(getParametericFilterParams<4>(sampleRate), onRealTimeThread, sampleRate);
    initializeChain<5>(getParametericFilterParams<5>(sampleRate), onRealTimeThread, sampleRate);
    initializeChain<6>(getParametericFilterParams<6>(sampleRate), onRealTimeThread, sampleRate);
    
    
    //low cut filter, and then high cut
    HighCutLowCutParameters lowCutParams = getCutFilterParams<0>(sampleRate, true);
    initializeChain<0>(lowCutParams,onRealTimeThread,sampleRate);
    HighCutLowCutParameters highCutParams = getCutFilterParams<7>(sampleRate, false);
    initializeChain<7>(highCutParams,onRealTimeThread,sampleRate);
 
}


void ParametricEQAudioProcessor::performPreLoopUpdate(double sampleRate)
{
    preUpdateCutFilter<0>(sampleRate, true);
    preUpdateParametricFilter<1>(sampleRate);
    preUpdateParametricFilter<2>(sampleRate);
    preUpdateParametricFilter<3>(sampleRate);
    preUpdateParametricFilter<4>(sampleRate);
    preUpdateParametricFilter<5>(sampleRate);
    preUpdateParametricFilter<6>(sampleRate);
    preUpdateCutFilter<7>(sampleRate, false);
}

void ParametricEQAudioProcessor::performInnerLoopUpdate(double sampleRate, int numSamplesToSkip)
{
    loopUpdateCutFilter<0>(sampleRate, true, numSamplesToSkip);
    loopUpdateParametricFilter<1>(sampleRate, numSamplesToSkip);
    loopUpdateParametricFilter<2>(sampleRate, numSamplesToSkip);
    loopUpdateParametricFilter<3>(sampleRate, numSamplesToSkip);
    loopUpdateParametricFilter<4>(sampleRate, numSamplesToSkip);
    loopUpdateParametricFilter<5>(sampleRate, numSamplesToSkip);
    loopUpdateParametricFilter<6>(sampleRate, numSamplesToSkip);
    loopUpdateCutFilter<7>(sampleRate, false, numSamplesToSkip);
}
