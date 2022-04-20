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
    
    // initialize filters first.
    FilterParameters paramatricParams = getParametericFilterParams<1>(sampleRate);
    leftChain.get<1>().initialize(paramatricParams, 0.0, false, sampleRate);
    
    //TODO replace these with initialize stuff.
    updateCutFilter<0>(sampleRate, true, oldHighCutParams, true);
    updateCutFilter<2>(sampleRate, true, oldLowCutParams, false);
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
    addFilterParamToLayout(layout, 2, true);
    
    return layout;
}



 



void ParametricEQAudioProcessor::updateFilters(double sampleRate, bool forceUpdate)
{
    updateCutFilter<0>(sampleRate, forceUpdate, oldHighCutParams, true);
    updateParametricFilter<1>(sampleRate);
    updateCutFilter<2>(sampleRate, forceUpdate, oldLowCutParams, false);
}
