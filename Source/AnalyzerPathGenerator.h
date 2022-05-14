/*
  ==============================================================================

    AnalyzerPathGenerator.h
    Created: 14 May 2022 1:23:57am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Fifo.h"

#define PATH_FIFO_DEPTH 100

struct AnalyzerPathGenerator
{
    /*
     converts 'renderData[]' into a juce::Path
     */
    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      size_t fftSize,
                      float binWidth,
                      float negativeInfinity = -60.f,
                      float maxDb = 12.f);
    
    size_t getNumPathsAvailable() const;
    bool getPath(juce::Path& path);
private:
    Fifo<juce::Path, PATH_FIFO_DEPTH> pathFifo;
};
