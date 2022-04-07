/*
  ==============================================================================

    Decibel.h
    Created: 6 Apr 2022 2:23:13pm
    Author:  Ronald Legere

  ==============================================================================
*/


#pragma once
#include <JuceHeader.h>

template <typename FloatType>
class Decibel {

public:
    Decibel() : decibels{0.0}
    {
    }
    
    explicit Decibel(FloatType dB) : decibels{dB}
    {
    }
    
    FloatType getDb() const
    {
        return decibels;
    }
    
    FloatType getGain() const
    {
        return juce::Decibels::decibelsToGain(decibels);
    }
    
    void setGain(FloatType g)
    {
        decibels = juce::Decibels::gainToDecibels(g);
    }
    
    void setDb(FloatType db)
    {
        decibels = db;
    }
    
    Decibel& operator+=(const Decibel& rhs)
    {
        decibels += rhs.decibels;
        return *this;
    }
    
    Decibel& operator-=(const Decibel& rhs)
    {
        decibels -= rhs.decibels;
        return *this;
    }
    
    friend Decibel operator+(Decibel lhs, const Decibel& rhs)
    {
        lhs += rhs;
        return lhs;
    }
    
    friend Decibel operator-(Decibel lhs, const Decibel& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
    
    friend bool operator< (const Decibel& lhs, const Decibel& rhs)
    {
        return lhs.decibels < rhs.decibels;
    }
    
    friend bool operator> (const Decibel& lhs, const Decibel& rhs)
    {
        return rhs < lhs;
    }
    
    friend bool operator<=(const Decibel& lhs, const Decibel& rhs)
    {
        return !(lhs > rhs);
    }
    
    friend bool operator>=(const Decibel& lhs, const Decibel& rhs)
    {
        return !(lhs < rhs);
    }
    
    friend bool operator==(const Decibel& lhs, const Decibel& rhs)
    {
        return lhs.decibels == rhs.decibels;
    }
    
    friend bool operator!=(const Decibel& lhs, const Decibel& rhs)
    {
        return !(lhs == rhs);
    }
    
private:
    FloatType decibels;
};
