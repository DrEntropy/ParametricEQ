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
                                         size_t fftSize, float binWidth,
                                         float negativeInfinity,
                                         float maxDb)
{
    juce::Path fftPath;
    
    auto startX = fftBounds.getX();
    auto endX  = startX + fftBounds.getWidth();
    auto height = fftBounds.getHeight();
    auto topY = fftBounds.getY();
    
    size_t numBins = fftSize/2;
    float maxLogFreq = std::log(20000.f);
    float minLogFreq = std::log(20.0f);
    
    // skip dc component
    auto y = juce::jmap(renderData[1], negativeInfinity, maxDb, topY + height, topY);
    auto x = juce::jmap(std::log(binWidth), minLogFreq, maxLogFreq, startX, endX);
    fftPath.startNewSubPath(x, y);
    
    auto prevX = startX;
     
    for(size_t i = 2; i <= numBins; ++i)
    {
        y = juce::jmap(renderData[i], negativeInfinity, maxDb, topY + height, topY);
        
        auto logFreq = std::log(i * binWidth);
 
         x = juce::jmap(logFreq, minLogFreq, maxLogFreq, startX, endX);
        
        // only draw one bin per x in the GUI.
        if(x - prevX > 1.f)
        {
            fftPath.lineTo(x, y);
            prevX = x;
        }
    }
    
    pathFifo.push(fftPath);
}

size_t AnalyzerPathGenerator::getNumPathsAvailable() const
{
    return pathFifo.getNumAvailableForReading();
}

bool AnalyzerPathGenerator::getPath(juce::Path& path)
{
    return pathFifo.pull(path);
}