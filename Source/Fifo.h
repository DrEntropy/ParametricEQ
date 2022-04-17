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
    }
    
    //used when T is AudioBuffer<float>
    void prepare (int numSamples, int numChannels)
    {
        static_assert (std::is_same<juce::AudioBuffer<float>, T>::value,
                      "Fifo::prepare(2 params) requires T to be AudioBuffer<float>!");
        for (auto& audioBuffer : buffer)
        {
            // don't bother clearing extra space, we are going to clear right after this call.
            audioBuffer.setSize (numChannels, numSamples, false, false, true);
            audioBuffer.clear();
        }
    }
    
    //used when T is std::vector<float>
    void prepare (size_t numElements)
    {
        static_assert (std::is_same<std::vector<float>, T>::value,
                      "Fifo::prepare(1 param) requires T to be vector<float>!");
        for (auto& elemVector:buffer)
        {
            elemVector.clear();
            elemVector.resize(numElements,0.0f);
        }
    }
    


    bool push (const T &t)
    {
        auto writeHandle = fifo.write(1);
        
        if (writeHandle.blockSize1 < 1)
            return false;
    
        if constexpr (isReferenceCountedObjectPtr<T>::value)
        {
            // save a copy of the ptr currently in buffer if any, increasing reference count.
            auto tempT {buffer[writeHandle.startIndex1]};
            buffer[writeHandle.startIndex1] = t;

            // verify we are not about to delete the object that was at this index, if any!
            jassert (tempT.get() == nullptr || tempT.get()->getReferenceCount() != 1);
    
        }
        else
        {
            buffer[writeHandle.startIndex1] = t;
        }
        return true;
    }
    
    bool pull (T& t)
    {
        auto readHandle = fifo.read(1);
        if (readHandle.blockSize1 > 0)
        {
            // note, a copy is returned, make sure somewhere this is dealt with!
            t = buffer[readHandle.startIndex1];
            return true;
        }
        
        return false;
    }
    
    
    bool exchange(T&& t)
    {

            auto readHandle = fifo.read(1);
            if (readHandle.blockSize1 > 0)
            {
                T temp;
                
                if constexpr (isReferenceCountedObjectPtr<T>::value)
                {
                    temp = std::move(buffer[readHandle.startIndex1]);
                    jassert(! buffer[readHandle.startIndex1].get());
                    buffer[readHandle.startIndex1]= t;
                    t = temp;
                    return true;
                }
                
                if constexpr(isReferenceCountedArray<T>::value)
                {
                    temp = std::move(buffer[readHandle.startIndex1]);
                    jassert(buffer[readHandle.startIndex1].isEmpty());
                    buffer[readHandle.startIndex1]= t;
                    t = temp;
                    return true;
                }
                
                if constexpr(isVector<T>::value)
                {
                    if(t.size() == buffer[readHandle.startIndex1].size())
                    {
                        temp = std::move(buffer[readHandle.startIndex1]);
                        jassert(buffer[readHandle.startIndex1].empty());
                        buffer[readHandle.startIndex1]= t;
                        t = temp;
                        return true;
                    }
                    
                    return false;
                }
                
                if constexpr(isAudioBuffer<T>::value)
                {
                    if(t.getNumSamples() == buffer[readHandle.startIndex1].getNumSamples())
                    {
                        temp = std::move(buffer[readHandle.startIndex1]);
                        jassert(buffer[readHandle.startIndex1].getNumSamples()==0);
                        buffer[readHandle.startIndex1]= t;
                        t = temp;
                        return true;
                    }
                    
                    return false;
                }
               
                // blind swap
                temp = std::move(buffer[readHandle.startIndex1]);
                jassert(true); // detect reaching this case to see if we need to do something special
                buffer[readHandle.startIndex1]= t;
                t = temp;
                return true;
                
             
            }
            return false;

        
    }
    
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
    
    // helpers for type checking
    template <typename U>
    struct isReferenceCountedObjectPtr : std::false_type { };

    template <typename W>
    struct isReferenceCountedObjectPtr<juce::ReferenceCountedObjectPtr<W>> : std::true_type { };
    
    template <typename U>
    struct isReferenceCountedArray : std::false_type { };

    template <typename W>
    struct isReferenceCountedArray<juce::ReferenceCountedArray<W>> : std::true_type { };
    
    template <typename U>
    struct isVector : std::false_type { };

    template <typename W>
    struct isVector<std::vector<W>> : std::true_type { };
    
    template <typename U>
    struct isAudioBuffer : std::false_type { };

    template <typename W>
    struct isAudioBuffer<juce::AudioBuffer<W>> : std::true_type { };
};
