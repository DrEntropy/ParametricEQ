/*
  ==============================================================================

    TestFunctions.cpp
    Created: 23 May 2022 1:52:29pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "TestFunctions.h"


float GetTestSignalFrequency(size_t binNum, size_t fftOrder, double sampleRate)
{
    auto fftSize = 1 << static_cast<int>(fftOrder);
    auto binWidthHz = static_cast<float>(sampleRate) / static_cast<float>(fftSize);
    auto numBins = fftSize / 2 + 1;
    
    jassert( juce::isPositiveAndBelow(binNum, numBins) );
    
    auto centerFreq =  binNum * binWidthHz;

    return centerFreq;
}
