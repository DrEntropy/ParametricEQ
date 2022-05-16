/*
  ==============================================================================

    PathProducer.h
    Created: 16 May 2022 9:49:48am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SingleChannelSampleFifo.h"
#include "FFTDataGenerator.h"
#include "AnalyzerPathGenerator.h"
#include "EQConstants.h"


template<typename BlockType>
struct PathProducer : juce::Thread
{
    PathProducer(double sr, SingleChannelSampleFifo<BlockType>&);
    ~PathProducer() override;
    
    void run() override;
    void changeOrder(FFTOrder o);
    int getFFTSize() const;
    double getBinWidth() const;
    void pauseThread();
    void setFFTRectBounds(juce::Rectangle<float>);
    
    void setDecayRate(float dr);
    bool pull(juce::Path&);
    int getNumAvailableForReading() const;
    void toggleProcessing(bool);
    void changePathRange(float negativeInfinityDb, float maxDb);
    
private:
    SingleChannelSampleFifo<BlockType>* singleChannelSampleFifo;
    FFTDataGenerator fftDataGenerator;
    AnalyzerPathGenerator pathGenerator;
    
    size_t getNumBins();
    
    std::vector<float> renderData;
    
    void updateRenderData(std::vector<float>& renderData,
                          const std::vector<float>& fftData,
                          int numBins,
                          float decayRate);
    
    BlockType bufferForGenerator;
    
    double sampleRate;
    juce::Rectangle<float> fftBounds;
    /*
     This must be atomic because it's used in inside `run()` as well as 'setDecayRate()' which can be called from any thread.
     */
    std::atomic<float> decayRateInDbPerSec { 0.f },
                       negativeInfinity { NEGATIVE_INFINITY },
                       maxDecibels { MAX_DECIBELS };
    /*
     this flag is toggled by the AnalyzerControls (indirectly) on/off button
     and causes processing to be skipped when it's false.
     */
    std::atomic<bool> processingIsEnabled { true };
};
