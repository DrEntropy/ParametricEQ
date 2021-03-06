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
    float maxLogFreq = std::log(MAX_FREQ);
    float minLogFreq = std::log(MIN_FREQ);
    
    auto mapX = [&](size_t i)
        {
           return juce::jmap(std::log(i * binWidth), minLogFreq, maxLogFreq, startX, endX);
        };
    
    auto mapY = [&](float gain)
        {
            return juce::jmap(gain, negativeInfinity, maxDb, topY + height, topY);
        };
    

    auto x = mapX(1);
    auto y = mapY(renderData[1]);
    
    if(x > startX)
    {
        // interpolate for x=startX
        auto index = MIN_FREQ / binWidth;
        x = startX;
        auto gain = index * renderData[1] + (1 - index) * renderData[0];
        y = mapY(gain);
    }
    
    fftPath.startNewSubPath(x, y);
    
    auto prevX = x;
    auto minY = y;
    bool findMin = false;
     
    for(size_t i = 2; i <= numBins; ++i)
    {
        x = mapX(i);
        y = mapY(renderData[i]);
        
        if(findMin && y < minY)
            minY = y;
        
        // only draw one bin per x in the GUI, and draw the highest (smallest y)
        if(x - prevX > 1.f)
        {
            fftPath.lineTo(x, findMin ? minY : y);
            prevX = x;
            findMin = false;
        }
        else if(! findMin)  // change state to find the highest until ready to draw again
        {
            findMin = true;
            minY = y;
        }
        
        
        if(x > endX)
            break;
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
