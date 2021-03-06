/*
  ==============================================================================

    PathProducer.cpp
    Created: 16 May 2022 9:49:48am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "PathProducer.h"


#define LOOP_DELAY 10


template<typename BlockType>
PathProducer<BlockType>::PathProducer(double sr, SingleChannelSampleFifo<BlockType>& fifoRef) : Thread("PathProducer"), singleChannelSampleFifo{&fifoRef}, sampleRate{sr} { }
 
template<typename BlockType>
PathProducer<BlockType>::~PathProducer()
{
    stopThread(2000);
}

template<typename BlockType>
void PathProducer<BlockType>::run()
{
    BlockType buffer;
    
    previousTime = juce::Time::currentTimeMillis();
    
    while(!threadShouldExit())
    {
        if(!processingIsEnabled)
        {
            wait(LOOP_DELAY);
            continue;
        }
        
        size_t fftSize = getFFTSize();
        auto BFGSize = bufferForGenerator.getNumSamples();
        jassert(fftSize == BFGSize);
        
        while(!threadShouldExit() && singleChannelSampleFifo->getNumCompleteBuffersAvailable() > 0)
        {
            auto success = singleChannelSampleFifo->getAudioBuffer(buffer);
            jassert(success);
            juce::ignoreUnused(success);
            
            auto SCSFSize = buffer.getNumSamples();
            jassert(SCSFSize <= BFGSize && BFGSize % SCSFSize == 0); //Spec requirement
            
            if(BFGSize > SCSFSize)
            {
                auto writePointer = bufferForGenerator.getWritePointer(0);
                auto readPointer = bufferForGenerator.getReadPointer(0);
                std::copy(readPointer + SCSFSize, readPointer + BFGSize, writePointer);
            }
            
            // copy SCSF into bufferForGenerator end.
            bufferForGenerator.copyFrom(0, BFGSize - SCSFSize, buffer, 0, 0, SCSFSize);
            
            fftDataGenerator.produceFFTDataForRendering(bufferForGenerator);
        }
        
        while(!threadShouldExit() && fftDataGenerator.getNumAvailableFFTDataBlocks() > 0)
        {
            std::vector<float> fftData;
            fftDataGenerator.getFFTData(fftData);
            
            auto deltaT = juce::Time::currentTimeMillis() - previousTime;
            previousTime += deltaT;
            
            updateRenderData(renderData, fftData, getNumBins(),  static_cast<float>(deltaT) * decayRateInDbPerSec.load() / 1000.f);
            pathGenerator.generatePath(renderData, fftBounds, fftSize, getBinWidth(), negativeInfinity, maxDecibels);
        }
 
        wait(LOOP_DELAY);
    }
}

template<typename BlockType>
void PathProducer<BlockType>::changeOrder(AnalyzerProperties::FFTOrder o)
{
    pauseThread();
    fftDataGenerator.changeOrder(o);
    renderData.assign(getNumBins() + 1, negativeInfinity.load());
    
    //prep bufferForGenerator
    bufferForGenerator.setSize (1, getFFTSize(), false, false, true);
    bufferForGenerator.clear();
    

    
    while(!singleChannelSampleFifo->isPrepared())
        wait(5);
    
    if(!fftBounds.isEmpty())
        startThread();
}

template<typename BlockType>
size_t PathProducer<BlockType>::getFFTSize() const
{
    return fftDataGenerator.getFFTSize();
}

template<typename BlockType>
double PathProducer<BlockType>::getBinWidth() const
{
    return sampleRate.load() / getFFTSize();
}

template<typename BlockType>
void PathProducer<BlockType>::pauseThread()
{
    auto stopped = stopThread(500);
    jassert(stopped);
    juce::ignoreUnused(stopped);
}

template<typename BlockType>
void PathProducer<BlockType>::setFFTRectBounds(juce::Rectangle<float> fft)
{
    pauseThread();
    if(!fft.isEmpty())
    {
        fftBounds = fft;
        startThread();
    }
    
}

template<typename BlockType>
void PathProducer<BlockType>::setDecayRate(float dr)
{
    decayRateInDbPerSec.store(dr);
}

template<typename BlockType>
bool PathProducer<BlockType>::pull(juce::Path& path)
{
   return pathGenerator.getPath(path);
}

template<typename BlockType>
size_t PathProducer<BlockType>::getNumAvailableForReading() const
{
    return pathGenerator.getNumPathsAvailable();
}

template<typename BlockType>
void PathProducer<BlockType>::toggleProcessing(bool enabled)
{
    processingIsEnabled.store(enabled);
}

template<typename BlockType>
void PathProducer<BlockType>::changePathRange(float negativeInfinityDb, float maxDb)
{
    maxDecibels.store(maxDb);
    negativeInfinity.store(negativeInfinityDb);
}

template<typename BlockType>
void PathProducer<BlockType>::updateRenderData(std::vector<float>& renderData,
                      const std::vector<float>& fftData,
                      int numBins,
                      float decayRate)
{
    
    jassert(decayRate >= 0.f);
    for(auto i=0; i <= numBins; ++i)
    {
        auto previousValue = renderData[i];
        auto candidate = fftData[i];
        auto finalValue = juce::jmax(candidate, previousValue - decayRate);
        renderData[i] = juce::jlimit(NEGATIVE_INFINITY, MAX_DECIBELS, finalValue);
        
    }
}


template<typename BlockType>
size_t PathProducer<BlockType>::getNumBins()
{
    return getFFTSize()/2;
}

template<typename BlockType>
void PathProducer<BlockType>::updateSampleRate(double sr)
{
    pauseThread();
    sampleRate.store(sr);
    if(!fftBounds.isEmpty())
        startThread();
 
}


template struct PathProducer< juce::AudioBuffer<float> >;
