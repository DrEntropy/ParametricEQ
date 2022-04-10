/*
  ==============================================================================

    Fifo.h
    Created: 7 Apr 2022 7:34:26pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template<typename T, size_t Size>
struct Fifo
{
    size_t getSize() const noexcept
    {
        return Size;
    };
    
    //used when T is AudioBuffer<float>
    void prepare(int numSamples, int numChannels);
    
    //used when T is std::vector<float>
    void prepare(size_t numElements);
    
    bool push(const T& t);
    bool pull(T& t);
    
    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }
    int getAvailableSpace() const
    {
        return fifo.getFreeSpace();
    }
private:
    juce::AbstractFifo fifo { Size };
    std::array<T, Size> buffer;
};
