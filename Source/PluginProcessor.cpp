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
    // reuse spec
    spec.numChannels = 2;
    inputTrim.prepare(spec);
    outputTrim.prepare(spec);
    
    
    initializeFilters(Channel::Left, sampleRate);
    initializeFilters(Channel::Right, sampleRate);
    
    inputBuffers.prepare(samplesPerBlock, getTotalNumInputChannels());
    
#ifdef USE_TEST_OSC
    testOsc.prepare(spec);
    testOsc.setFrequency(440.0f);
    testOscGain.prepare(spec);
#endif

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

void ParametricEQAudioProcessor::performMidSideTransform(juce::AudioBuffer<float>& buffer)
{
    // AKA Hadamard transformation
    // Anew = (A+B)/sqrt(2), Bnew = (A-B)/sqrt(2)
    // note that this is involutory , i.e. a second call will undo the transformation
    
    static const float minus3db = 1.0f/juce::MathConstants<float>::sqrt2;

    auto leftReadPtr = buffer.getReadPointer(0);
    auto rightReadPtr = buffer.getReadPointer(1);

    auto leftWritePtr = buffer.getWritePointer(0);
    auto rightWritePtr = buffer.getWritePointer(1);
    
    int numSamples = buffer.getNumSamples();

    for( int i=0; i < numSamples; i++ )
    {
         auto M = (leftReadPtr[i] + rightReadPtr[i]) * minus3db;
         auto S = (leftReadPtr[i] - rightReadPtr[i]) * minus3db;

         leftWritePtr[i] = M;
         rightWritePtr[i] = S;
    }
}

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
    
    updateTrims();
    
    ChannelMode mode = static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load());
    
    performPreLoopUpdate(mode, getSampleRate());
    
    juce::dsp::AudioBlock<float> block(buffer);
    
    int numSamples = buffer.getNumSamples();
    int offset = 0;
    
    juce::dsp::ProcessContextReplacing<float> stereoContext(block);
    inputTrim.process(stereoContext);
    
#ifdef USE_TEST_OSC
    for( auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);
    
    testOscGain.setGainDecibels(JUCE_LIVE_CONSTANT(0.0f));
    for( auto j = 0; j < numSamples; ++j)
        {
            auto sample = testOsc.processSample(0.0f);
            sample = testOscGain.processSample(sample);
            buffer.setSample(0, j, sample);
            buffer.setSample(1, j, sample);
        }
#endif
    
    inputBuffers.push(buffer);
    
    updateMeterFifos(inMeterValuesFifo, buffer);
 
    if(mode == ChannelMode::MidSide)
    {
        performMidSideTransform(buffer);
    }
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    while(offset < numSamples)
    {
        int blockSize = std::min(numSamples - offset, innerLoopSize);
        auto leftSubBlock =  leftBlock.getSubBlock(offset, blockSize);
        auto rightSubBlock = rightBlock.getSubBlock(offset, blockSize);
 
        performInnerLoopUpdate(blockSize);
        
        juce::dsp::ProcessContextReplacing<float> leftContext(leftSubBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightSubBlock);
        leftChain.process(leftContext);
        rightChain.process(rightContext);
                
        offset += innerLoopSize;
    }
    
    if(mode == ChannelMode::MidSide)
    {
        performMidSideTransform(buffer);
    }
    
#ifdef USE_TEST_OSC
    for( auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
#endif
    
    outputTrim.process(stereoContext);
    updateMeterFifos(outMeterValuesFifo, buffer);
}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
   return new ParametricEQAudioProcessorEditor (*this);
   // return new  juce::GenericAudioProcessorEditor(*this);
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
    initializeFilters(Channel::Left, getSampleRate());
    initializeFilters(Channel::Right, getSampleRate());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}

void ParametricEQAudioProcessor::addFilterParamToLayout (ParamLayout& layout,Channel channel, int filterNum, bool isCut)
{
    auto label = createBypassParamString(channel, filterNum);
    layout.add(std::make_unique<juce::AudioParameterBool>(label, label, true) );
    
    label = createFreqParamString(channel, filterNum);
    layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                       juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20.0f));
    
    
    if(!isCut)
    {
        label = createQParamString(channel, filterNum);
        layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.0f), 1.0f));
        
        label = createGainParamString(channel, filterNum);
        layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.0f), 0.0f));
        juce::StringArray types;
        
        for (const auto& [type, stringRep] : FilterInfo::mapFilterTypeToString)
        {
            types.add(stringRep);
        }
        label = createTypeParamString(channel, filterNum);
        layout.add(std::make_unique<juce::AudioParameterChoice>(label, label, types, 10));
    }
    else
    {
        label = createQParamString(channel, filterNum);
        layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 1.0f), 0.71f));
        juce::StringArray slopes;
        
        for (const auto& [order, stringRep] : FilterInfo::mapSlopeToString)
        {
            slopes.add(stringRep);
        }
        
        label = createSlopeParamString(channel, filterNum);
        layout.add(std::make_unique<juce::AudioParameterChoice>(label, label, slopes, 0));
    }
}

void ParametricEQAudioProcessor::createFilterLayouts(ParamLayout& layout, Channel channel)
{
    addFilterParamToLayout(layout, channel, 0, true);
    addFilterParamToLayout(layout, channel, 1, false);
    addFilterParamToLayout(layout, channel, 2, false);
    addFilterParamToLayout(layout, channel, 3, false);
    addFilterParamToLayout(layout, channel, 4, false);
    addFilterParamToLayout(layout, channel, 5, false);
    addFilterParamToLayout(layout, channel, 6, false);
    addFilterParamToLayout(layout, channel, 7, true);
}

// for some reason compiler will not let me use the alias for return type here.
ParamLayout ParametricEQAudioProcessor::createParameterLayout()
{
    ParamLayout layout;
    
     
    juce::StringArray modes;
    
    for (const auto& [mode, stringRep] : mapModeToString)
    {
        modes.add(stringRep);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("Processing Mode", "Processing Mode", modes, 0));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("input_trim", "input_trim",
                                                           juce::NormalisableRange<float>(-18.f, 18.f, 0.25f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("output_trim", "output_trim",
                                                           juce::NormalisableRange<float>(-18.f, 18.f, 0.25f, 1.0f), 0.0f));
    createFilterLayouts(layout, Channel::Left);
    createFilterLayouts(layout, Channel::Right);
 
    return layout;
}


void ParametricEQAudioProcessor::initializeFilters(Channel channel, double sampleRate)
{
    // check if on realtime thread
    auto messMan = juce::MessageManager::getInstanceWithoutCreating();
    bool onRealTimeThread=  ! ((messMan != nullptr) && messMan->isThisTheMessageThread());
    
    // initialize filters
   
    initializeChain<1>(getParametericFilterParams<1>(channel, sampleRate), onRealTimeThread, sampleRate);
    initializeChain<2>(getParametericFilterParams<2>(channel, sampleRate), onRealTimeThread, sampleRate);
    initializeChain<3>(getParametericFilterParams<3>(channel, sampleRate), onRealTimeThread, sampleRate);
    initializeChain<4>(getParametericFilterParams<4>(channel, sampleRate), onRealTimeThread, sampleRate);
    initializeChain<5>(getParametericFilterParams<5>(channel, sampleRate), onRealTimeThread, sampleRate);
    initializeChain<6>(getParametericFilterParams<6>(channel, sampleRate), onRealTimeThread, sampleRate);
    
    
    //low cut filter, and then high cut
    HighCutLowCutParameters lowCutParams = getCutFilterParams<0>(channel, sampleRate, true);
    initializeChain<0>(lowCutParams,onRealTimeThread,sampleRate);
    HighCutLowCutParameters highCutParams = getCutFilterParams<7>(channel, sampleRate, false);
    initializeChain<7>(highCutParams,onRealTimeThread,sampleRate);
 
}


void ParametricEQAudioProcessor::performPreLoopUpdate(ChannelMode mode, double sampleRate)
{
    preUpdateCutFilter<0>(mode, sampleRate, true);
    preUpdateParametricFilter<1>(mode, sampleRate);
    preUpdateParametricFilter<2>(mode, sampleRate);
    preUpdateParametricFilter<3>(mode, sampleRate);
    preUpdateParametricFilter<4>(mode, sampleRate);
    preUpdateParametricFilter<5>(mode, sampleRate);
    preUpdateParametricFilter<6>(mode, sampleRate);
    preUpdateCutFilter<7>(mode, sampleRate, false);
}

void ParametricEQAudioProcessor::performInnerLoopUpdate(int numSamplesToSkip)
{
    loopUpdateCutFilter<0>(numSamplesToSkip);
    loopUpdateParametricFilter<1>(numSamplesToSkip);
    loopUpdateParametricFilter<2>(numSamplesToSkip);
    loopUpdateParametricFilter<3>(numSamplesToSkip);
    loopUpdateParametricFilter<4>(numSamplesToSkip);
    loopUpdateParametricFilter<5>(numSamplesToSkip);
    loopUpdateParametricFilter<6>(numSamplesToSkip);
    loopUpdateCutFilter<7>(numSamplesToSkip);
}

void ParametricEQAudioProcessor::updateTrims()
{
    
    float inputGain= apvts.getRawParameterValue("input_trim")->load();
    float outputGain = apvts.getRawParameterValue("output_trim")->load();
    inputTrim.setGainDecibels(inputGain);
    outputTrim.setGainDecibels(outputGain);
 
}
