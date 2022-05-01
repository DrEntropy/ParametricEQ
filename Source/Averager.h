/*
  ==============================================================================

    Averager.h
    Created: 1 May 2022 8:12:36am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <atomic>

template<typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void resize(size_t numElements, T initialValue)
    {
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
        auto size = elements.size();
        
        if(size > 0)
        {
            avg.store(static_cast<float>(initialValue));
            sum.store(initialValue * elements.size());
        }
        else
        {
            avg.store(0.f);
            sum.store(T(0));
        }
    }
    
    size_t getSize() const
    {
        return elements.size();
    }
    
    void add(T t)
    {
        auto size = elements.size();
        if(size > 0)
        {
            float currentSum = sum.load();
            size_t currentWriteIndex = writeIndex.load();
            
            currentSum += t - elements[currentWriteIndex];
            elements[currentWriteIndex] = t;
            
            currentWriteIndex = (currentWriteIndex + 1) % size;

            sum.store(currentSum);
            avg.store(currentSum / size);
            writeIndex.store(currentWriteIndex);
        }
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
