/*
  ==============================================================================

    AnalyzerPathGenerator.cpp
    Created: 14 May 2022 1:23:57am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "AnalyzerPathGenerator.h"


void AnalyzerPathGenerator::generatePath(const std::vector<float>& renderData,
                  juce::Rectangle<float> fftBounds,
                  int fftSize,
                  float binWidth,
                  float negativeInfinity,
                  float maxDb)
{
    juce::Path fftPath;
    
    //iterate through fft array
}

int AnalyzerPathGenerator::getNumPathsAvailable() const
{
    return pathFifo.getNumAvailableForReading();
}

bool AnalyzerPathGenerator::getPath(juce::Path&& path)
{
    return pathFifo.exchange(std::move(path));
}
