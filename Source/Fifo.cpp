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



template <typename T, typename W>
struct isReferenceCountedObjectPtr : std::false_type { };

template <typename W>
struct isReferenceCountedObjectPtr<juce::ReferenceCountedObjectPtr<W>, W> : std::true_type { };

template <typename T, size_t Size>
void Fifo<T, Size>::push(<#const T &t#>)
{
    if constexpr (isReferenceCountedObjectPtr<T, W>::value)
    {
        //  deal with reference counting
    }
    else
    {
        //normal push
    }
}
