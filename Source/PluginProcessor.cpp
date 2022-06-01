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
#include "TestFunctions.h"
 
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
    
    
    initializeFilters(leftChain, Channel::Left, sampleRate);
    initializeFilters(rightChain, Channel::Right, sampleRate);
 
    leftSCSFifo.prepare(SCSF_SIZE);
    rightSCSFifo.prepare(SCSF_SIZE);
    
    sampleRateListeners.call([sampleRate](SampleRateListener& srl){srl.sampleRateChanged(sampleRate);});
 
    
    
#if USE_TEST_OSC
    testOsc.prepare(spec);
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
    
#if USE_TEST_OSC
    using namespace AnalyzerProperties;
    auto fftOrder = getFFTOrder(apvts.getParameter(getAnalyzerParamName(ParamNames::AnalyzerPoints))->getValue());
    auto fftSize = 1 << static_cast<int>(fftOrder);
    size_t numBins = fftSize / 2 + 1;

    
    auto centerFreq = GetTestSignalFrequency(std::min(binNum.load(), numBins), static_cast<size_t>(fftOrder),
                                             getSampleRate());
    

    testOsc.setFrequency(centerFreq);
    
    for( auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);
    
    //testOsc.setFrequency(JUCE_LIVE_CONSTANT(15000.f));
    
    testOscGain.setGainDecibels(JUCE_LIVE_CONSTANT(0.0f));
    for( auto j = 0; j < numSamples; ++j)
    {
        auto sample = testOsc.processSample(0.0f);
        sample = testOscGain.processSample(sample);
        buffer.setSample(0, j, sample);
        buffer.setSample(1, j, sample);
    }
#endif
    
    
    
#if USE_WHITE_NOISE
    
    for( auto j = 0; j < numSamples; ++j)
    {
        auto sample = random.nextFloat() * 2.0f - 1.0f;
        buffer.setSample(0, j, sample);
        buffer.setSample(1, j, sample);
    }

#endif
    using namespace AnalyzerProperties;
    
    auto analyzerEnabled = apvts.getRawParameterValue(getAnalyzerParamName(ParamNames::EnableAnalyzer))->load() > 0.;
    
    auto analyzerMode = static_cast<ProcessingModes> (apvts.getRawParameterValue(getAnalyzerParamName(ParamNames::AnalyzerProcessingMode))->load()) ;
    
    if(editorActive)
    {
        if(analyzerEnabled && analyzerMode == ProcessingModes::Pre)
        {
            leftSCSFifo.update(buffer);
            rightSCSFifo.update(buffer);
        }
        
        updateMeterFifos(inMeterValuesFifo, buffer);
    }
 
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
    

    
    outputTrim.process(stereoContext);
    
    if(editorActive)
    {
        if(analyzerEnabled && analyzerMode == ProcessingModes::Post)
        {
            leftSCSFifo.update(buffer);
            rightSCSFifo.update(buffer);
        }
        updateMeterFifos(outMeterValuesFifo, buffer);
    }
    
#if USE_TEST_OSC || USE_WHITE_NOISE
    //testOsc.setFrequency(JUCE_LIVE_CONSTANT(5000));
    for( auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
#endif
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
    initializeFilters(leftChain, Channel::Left, getSampleRate());
    initializeFilters(rightChain, Channel::Right, getSampleRate());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
 
void ParametricEQAudioProcessor::addFilterParamToLayout (ParamLayout& layout, Channel channel, ChainPosition chainPos, bool isCut)
{
    auto label = createBypassParamString(channel, chainPos);
    layout.add(std::make_unique<juce::AudioParameterBool>(label, label, true) );
    
    label = createFreqParamString(channel, chainPos);
    layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                       juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
                                                                     (chainPos == ChainPosition::HighCut ? 20000.0f : 20.0f)));
    
    
    if(!isCut)
    {
        label = createQParamString(channel, chainPos);
        layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.0f), 1.0f));
        
        label = createGainParamString(channel, chainPos);
        layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.0f), 0.0f));
    }
    else
    {
        label = createQParamString(channel, chainPos);
        layout.add(std::make_unique<juce::AudioParameterFloat>(label, label,
                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 1.0f), 0.71f));
        juce::StringArray slopes;
        
        for (const auto& [order, stringRep] : FilterInfo::mapSlopeToString)
        {
            slopes.add(stringRep);
        }
        
        label = createSlopeParamString(channel, chainPos);
        layout.add(std::make_unique<juce::AudioParameterChoice>(label, label, slopes, 0));
    }
}

void ParametricEQAudioProcessor::createFilterLayouts(ParamLayout& layout, Channel channel)
{
    addFilterParamToLayout(layout, channel, ChainPosition::LowCut, true);
    addFilterParamToLayout(layout, channel, ChainPosition::LowShelf, false);
    addFilterParamToLayout(layout, channel, ChainPosition::PeakFilter1, false);
    addFilterParamToLayout(layout, channel, ChainPosition::PeakFilter2, false);
    addFilterParamToLayout(layout, channel, ChainPosition::PeakFilter3, false);
    addFilterParamToLayout(layout, channel, ChainPosition::PeakFilter4, false);
    addFilterParamToLayout(layout, channel, ChainPosition::HighShelf, false);
    addFilterParamToLayout(layout, channel, ChainPosition::HighCut, true);
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
    
    AnalyzerProperties::addAnalyzerParams(layout);
    
    return layout;
}


void ParametricEQAudioProcessor::initializeFilters(ChainHelpers::MonoFilterChain& chain, Channel channel,  double sampleRate)
{
    // check if on realtime thread
    auto messMan = juce::MessageManager::getInstanceWithoutCreating();
    bool onRealTimeThread=  ! ((messMan != nullptr) && messMan->isThisTheMessageThread());
    
    using namespace ChainHelpers;
    // initialize filters
   
    initializeChainLink<ChainPosition::LowShelf, FilterParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    initializeChainLink<ChainPosition::PeakFilter1, FilterParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    initializeChainLink<ChainPosition::PeakFilter2, FilterParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    initializeChainLink<ChainPosition::PeakFilter3, FilterParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    initializeChainLink<ChainPosition::PeakFilter4, FilterParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    initializeChainLink<ChainPosition::HighShelf, FilterParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    
    
    //low cut filter, and then high cut
   
    initializeChainLink<ChainPosition::LowCut, HighCutLowCutParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
    initializeChainLink<ChainPosition::HighCut, HighCutLowCutParameters>(chain, channel, apvts, rampTime, onRealTimeThread, sampleRate);
}


void ParametricEQAudioProcessor::performPreLoopUpdate(ChannelMode mode, double sampleRate)
{
    preUpdateCutFilter<ChainPosition::LowCut>(mode, sampleRate);
    preUpdateParametricFilter<ChainPosition::LowShelf>(mode, sampleRate);
    preUpdateParametricFilter<ChainPosition::PeakFilter1>(mode, sampleRate);
    preUpdateParametricFilter<ChainPosition::PeakFilter2>(mode, sampleRate);
    preUpdateParametricFilter<ChainPosition::PeakFilter3>(mode, sampleRate);
    preUpdateParametricFilter<ChainPosition::PeakFilter4>(mode, sampleRate);
    preUpdateParametricFilter<ChainPosition::HighShelf>(mode, sampleRate);
    preUpdateCutFilter<ChainPosition::HighCut>(mode, sampleRate);
}

void ParametricEQAudioProcessor::performInnerLoopUpdate(int numSamplesToSkip)
{
    loopUpdateCutFilter<ChainPosition::LowCut>(numSamplesToSkip);
    loopUpdateParametricFilter<ChainPosition::LowShelf>(numSamplesToSkip);
    loopUpdateParametricFilter<ChainPosition::PeakFilter1>(numSamplesToSkip);
    loopUpdateParametricFilter<ChainPosition::PeakFilter2>(numSamplesToSkip);
    loopUpdateParametricFilter<ChainPosition::PeakFilter3>(numSamplesToSkip);
    loopUpdateParametricFilter<ChainPosition::PeakFilter4>(numSamplesToSkip);
    loopUpdateParametricFilter<ChainPosition::HighShelf>(numSamplesToSkip);
    loopUpdateCutFilter<ChainPosition::HighCut>(numSamplesToSkip);
}

void ParametricEQAudioProcessor::updateTrims()
{
    
    float inputGain= apvts.getRawParameterValue("input_trim")->load();
    float outputGain = apvts.getRawParameterValue("output_trim")->load();
    inputTrim.setGainDecibels(inputGain);
    outputTrim.setGainDecibels(outputGain);
 
}



bool ParametricEQAudioProcessor::isAnyActiveOn()
{
    ChannelMode mode = static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load());
    
    bool isAnyOn = false;
    
    for(int filterNum = 0; filterNum < 8; ++filterNum)
    {
        bool isOn = apvts.getRawParameterValue(createBypassParamString(Channel::Left, static_cast<ChainPosition>(filterNum)))->load() < 0.5f;
        isAnyOn |= isOn;
        if(mode != ChannelMode::Stereo)
        {
            isOn = apvts.getRawParameterValue(createBypassParamString(Channel::Right, static_cast<ChainPosition>(filterNum)))->load() < 0.5f;
            isAnyOn |= isOn;
        }
    }
     
    return isAnyOn;
}


void ParametricEQAudioProcessor::setBypassed(bool state)
{
    for(int filterNum = 0; filterNum < 8; ++filterNum)
    {
        auto leftBypass = dynamic_cast<juce::AudioParameterBool *> (apvts.getParameter(createBypassParamString(Channel::Left,
                                                                                                               static_cast<ChainPosition>(filterNum))));
        auto rightBypass = dynamic_cast<juce::AudioParameterBool *> (apvts.getParameter(createBypassParamString(Channel::Right,
                                                                                                                static_cast<ChainPosition>(filterNum))));
        setBoolParamState(state, leftBypass);
        setBoolParamState(state, rightBypass);
    }    
}


void ParametricEQAudioProcessor::setBoolParamState(bool state, juce::AudioParameterBool* param)
{
    param -> beginChangeGesture();
    *param = state;
    param -> endChangeGesture();
}


void ParametricEQAudioProcessor::addSampleRateListener (SampleRateListener* srl)
{
    sampleRateListeners.add(srl);
}


void ParametricEQAudioProcessor::removeSampleRateListener (SampleRateListener* srl)
{
    jassert(sampleRateListeners.contains(srl));
    sampleRateListeners.remove(srl);
}
