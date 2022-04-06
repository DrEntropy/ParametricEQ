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

template <typename FloatType>
class Decibel {

public:
    Decibel() : decibels{0.0}
    {
    }
    explicit Decibel(FloatType dB) : decibels{dB}
    {
    }
    
    
    
private:
    FloatType decibels;
};
