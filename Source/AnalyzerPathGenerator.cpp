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
                  size_t fftSize,
                  float binWidth,
                  float negativeInfinity,
                  float maxDb)
{
    juce::Path fftPath;
    
    auto startX = fftBounds.getX();
    auto width  = fftBounds.getWidth();
    auto height = fftBounds.getHeight();
    auto topY = fftBounds.getY();  // the op.
    
    auto y = juce::jmap(renderData[0], negativeInfinity, maxDb, topY, topY - height);
    
    fftPath.startNewSubPath(startX, y);
    
    size_t numBins = fftSize/2;
    float maxLogFreq = std::log(static_cast<float>( numBins ));
    
    auto prevX = startX;
    
    // not that last bin at N/2+1 is the nyquist bin
    for(size_t i = 1; i <= numBins; ++i)
    {
        y = juce::jmap(renderData[i], negativeInfinity, maxDb, topY, topY - height);
        
        auto logFreq = std::log(static_cast<float>( i ));
        
         // only draw one bin per x in the GUI.  As you progress from left to right, the width of each bin becomes more narrow in terms of pixels.
         // If you compute the x position of the bin’s center frequency, you can determine if it is overlapping the previous bin’s x coordinate or not.
        float x = juce::jmap(logFreq, 0.f, maxLogFreq, startX, startX + width);
        if(x - prevX > 1.f)
            fftPath.lineTo(x, y);
        
        prevX = x;
    }
    
    pathFifo.push(fftPath);
}

size_t AnalyzerPathGenerator::getNumPathsAvailable() const
{
    return pathFifo.getNumAvailableForReading();
}

bool AnalyzerPathGenerator::getPath(juce::Path&& path)
{
    return pathFifo.exchange(std::move(path));
}
