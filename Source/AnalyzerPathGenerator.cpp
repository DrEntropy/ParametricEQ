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
    
    float x,y;
    
    auto mapXY = [&](size_t i)
        {
            x = juce::jmap(std::log(i * binWidth), minLogFreq, maxLogFreq, startX, endX);
            y = juce::jmap(renderData[i], negativeInfinity, maxDb, topY + height, topY);
        };

    mapXY(1);
    
    if(x > startX)
    {
        // interpolate for x=startX
        auto index = 20.f/binWidth;
        x = startX;
        auto gain = index * renderData[1] + (1 - index) * renderData[0];
        y = juce::jmap(gain, negativeInfinity, maxDb, topY + height, topY);
    }
    
    fftPath.startNewSubPath(x, y);
    
    auto prevX = x;
     
    for(size_t i = 2; i <= numBins; ++i)
    {
        mapXY(i);
        // only draw one bin per x in the GUI.
        if(x - prevX > 1.f)
        {
            fftPath.lineTo(x, y);
            prevX = x;
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
