/*
  ==============================================================================

    Fifo.cpp
    Created: 7 Apr 2022 7:34:26pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "Fifo.h"

template <typename T, size_t Size>
void Fifo<T, Size>::prepare(int numSamples, int numChannels)
{
    static_assert(std::is_same<juce::AudioBuffer<float>, T>::value,"Fifo::prepare(2 params) requires T to be AudioBuffer<float>!");
    for (auto& audioBuffer:buffer)
    {
        // don't bother clearing extra space, we are going to clear right after this call.
        audioBuffer.setSize(numChannels, numSamples, false, false, true);
        audioBuffer.clear();
    }
}

template <typename T, size_t Size>
void Fifo<T, Size>::prepare(size_t numElements)
{
    static_assert(std::is_same<std::vector<float>, T>::value,"Fifo::prepare(1 param) requires T to be vector<float>!");
    for (auto& elemVector:buffer)
    {
        elemVector.clear();
        elemVector.resize(numElements,0.0f);
    }
}



template <typename T>
struct isReferenceCountedObjectPtr : std::false_type { };

template <typename W>
struct isReferenceCountedObjectPtr<juce::ReferenceCountedObjectPtr<W> > : std::true_type { };

template <typename T, size_t Size>
bool Fifo<T, Size>::push(const T &t)
{
    auto writeHandle = fifo.write(1);
    
    if (writeHandle.blocksize1 < 1)
        return false;
    
    if constexpr (isReferenceCountedObjectPtr<T>::value)
    {
        // save a copy of the ptr currently in buffer, increasing reference count.
        auto tempT {buffer[writeHandle.startIndex1]};
        buffer[writeHandle.startIndex1] = t;
        
        // verify we are not about to delete the object that was at this index.
        jassert(tempT.get()->getReferenceCount() > 1);
        
        return true;
    }
    buffer[writeHandle.startIndex1] = t;
    return true;
}

template <typename T, size_t Size>
bool Fifo<T, Size>::pull(T &t)
{
    auto readHandle = fifo.read(1);
    if (readHandle.blocksize > 1)
    {
        t = buffer[readHandle.startIndex1];
        return true;
    }
    return false;
}
 
