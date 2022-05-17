/*
  ==============================================================================

    PathProducer.cpp
    Created: 16 May 2022 9:49:48am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "PathProducer.h"

#define SCSF_SIZE 2048
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
        
        BlockType tempBuffer(1, BFGSize);
        
        while(!threadShouldExit() && singleChannelSampleFifo->getNumCompleteBuffersAvailable() > 0)
        {
            auto success = singleChannelSampleFifo->getAudioBuffer(buffer);
            jassert(success);
            
            auto SCSFSize = buffer.getNumSamples();
            jassert(SCSFSize <= BFGSize);
            
            // copy shifted. Note if BFGSize == SCSFSize nothing happens.
            tempBuffer.copyFrom(0, 0, bufferForGenerator, 0, SCSFSize, BFGSize - SCSFSize);
            
            // copy SCSF into tempBuffer's end
            tempBuffer.copyFrom(0, BFGSize - SCSFSize, buffer, 0, 0, SCSFSize);
            
            std::swap(tempBuffer, bufferForGenerator);
            
            fftDataGenerator.produceFFTDataForRendering(bufferForGenerator);
        }
        
        while(!threadShouldExit() && fftDataGenerator.getNumAvailableFFTDataBlocks() > 0)
        {
            std::vector<float> fftData;
            fftDataGenerator.getFFTData(std::move(fftData));
            updateRenderData(renderData, fftData, getNumBins(),  static_cast<float>(LOOP_DELAY) * decayRateInDbPerSec.load() / 1000.f);
            pathGenerator.generatePath(renderData, fftBounds, fftSize, getBinWidth());
        }
 
        wait(LOOP_DELAY);
    }
}

template<typename BlockType>
void PathProducer<BlockType>::changeOrder(FFTOrder o)
{
    pauseThread();
    fftDataGenerator.changeOrder(o);
    renderData.assign(getNumBins() + 1, negativeInfinity.load());
    
    //prep bufferForGenerator
    bufferForGenerator.setSize (1, getFFTSize(), false, false, true);
    bufferForGenerator.clear();
    
    //prep SCSF to fixed size
    singleChannelSampleFifo->prepare(SCSF_SIZE);
    
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
    return sampleRate / getFFTSize();
}

template<typename BlockType>
void PathProducer<BlockType>::pauseThread()
{
       stopThread(2000);  
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
        renderData[i] = juce::jlimit(negativeInfinity.load(), maxDecibels.load(), finalValue);
    }
}


template<typename BlockType>
size_t PathProducer<BlockType>::getNumBins()
{
    return getFFTSize()/2;
}



template struct PathProducer< juce::AudioBuffer<float> >;
