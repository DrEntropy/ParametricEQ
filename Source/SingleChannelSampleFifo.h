/*
  ==============================================================================

    SingleChannelSampleFifo.h
    Created: 13 May 2022 1:39:57pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Fifo.h"
#include "ParameterHelpers.h"

#define SINGLE_CHANNEL_BUFFER_FIFO_DEPTH 100

template<typename BlockType>
struct SingleChannelSampleFifo
{
    using SampleType = typename BlockType::SampleType;
    
    SingleChannelSampleFifo(Channel ch) : channelToUse {ch}, prepared {false} {}
    
    void update(const BlockType& buffer)
    {
        if(!prepared.get())
            return;
        
        if (buffer.getNumChannels() > 0)
        {
            auto* channelData = buffer.getReadPointer (static_cast<int>(channelToUse));
 
            for (auto i = 0; i < buffer.getNumSamples(); ++i)
                pushNextSampleIntoFifo (channelData[i]);
        }
    }
    
    void pushNextSampleIntoFifo(SampleType sample)
    {
        if (fifoIndex == size.get())
        {
            bool success = audioBufferFifo.push(bufferToFill);
            jassert(success);
            juce::ignoreUnused(success);
            fifoIndex = 0;
        }
        
        auto writePointer = bufferToFill.getWritePointer(0);
    
        jassert(writePointer);
    
        writePointer[fifoIndex++] = sample;
    }
    
    void prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);
        fifoIndex = 0;
        bufferToFill.setSize(1, bufferSize, false, false, true);
        audioBufferFifo.prepare(bufferSize,0);
        prepared.set(true);
    }
    
    int getNumCompleteBuffersAvailable() const
    {
        if(! prepared.get())
            return 0;
        
        return audioBufferFifo.getNumAvailableForReading();
    }
    
    bool getAudioBuffer(BlockType& buf)
    {
        return audioBufferFifo.pull(buf);
    }
    
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
    
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType, SINGLE_CHANNEL_BUFFER_FIFO_DEPTH> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;
};
