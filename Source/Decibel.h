/*
  ==============================================================================

    Decibel.h
    Created: 6 Apr 2022 2:23:13pm
    Author:  Ronald Legere

  ==============================================================================
*/

/*
Rule of 3
math operators for when this object is on the left side of the +=, -=, etc
math operators for when this object is on the right side of the =: auto v = gain1 + gain2;   These should be friend functions
comparison operators.  these should be friend functions
 
FloatType getGain() const
FloatType getDb() const
setGain(FloatType g)
setDb(FloatType db)
 
2 constructors:
default constructor
a constructor that takes a FloatType parameter to initialize your member variable.
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
