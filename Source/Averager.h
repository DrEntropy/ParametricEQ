/*
  ==============================================================================

    Averager.h
    Created: 1 May 2022 8:12:36am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

template<typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue);
    
    void resize(size_t numElements, T initialValue);
    
    void clear(T initialValue);
    
    size_t getSize() const;
    
    void add(T t);
    
    float getAvg() const;
private:
    std::vector<T> elements;
    std::atomic<float> avg { static_cast<float>( T() ) };
    std::atomic<size_t> writeIndex = 0;
    std::atomic<T> sum { 0 };
};
