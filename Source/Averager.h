/*
  ==============================================================================

    Averager.h
    Created: 1 May 2022 8:12:36am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <atomic>
#include <JuceHeader.h>

template<typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void resize(size_t numElements, T initialValue)
    {
        jassert(numElements > 0);
        elements.resize(numElements);
        clear(initialValue);
    }
    
    void clear(T initialValue)
    {
        for( auto& value : elements)
        {
            value = initialValue;
        }

        writeIndex.store(0);
        avg.store(static_cast<float>(initialValue));
        sum.store(initialValue * getSize());
    }
    
    size_t getSize() const
    {
        return elements.size();
    }
    
    void add(T t)
    {
        auto size = getSize();
        float currentSum = sum.load();
        size_t currentWriteIndex = writeIndex.load();
            
        currentSum += t - elements[currentWriteIndex];
        elements[currentWriteIndex] = t;
            
        currentWriteIndex = (currentWriteIndex + 1) % size;

        sum.store(currentSum);
        avg.store(currentSum / size);
        writeIndex.store(currentWriteIndex);
    }
    
    float getAvg() const
    {
        return avg.load();
    }
    
private:
    std::vector<T> elements;
    std::atomic<float> avg { static_cast<float>( T() ) };
    std::atomic<size_t> writeIndex { 0 };
    std::atomic<T> sum { 0 };
};
