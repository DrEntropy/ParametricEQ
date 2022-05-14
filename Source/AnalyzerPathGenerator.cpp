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
    
    auto y = juce::jmap(renderData[0], negativeInfinity, maxDb, topY + height, topY);
    
    fftPath.startNewSubPath(startX, y);
    
    size_t numBins = fftSize/2;
    float maxLogFreq = std::log(static_cast<float>( numBins ));
    
  
    
    // make space from dc to first bin same as from first bin to second bin.
    // in the end we probably dont want to draw the dc component?
    startX = juce::jmap(std::log(2.f), std::log(1.f), maxLogFreq, startX, startX + width);
    
    
    auto prevX = startX;
     
    for(size_t i = 1; i <= numBins; ++i)
    {
        y = juce::jmap(renderData[i], negativeInfinity, maxDb, topY + height, topY);
        
        auto logFreq = std::log(static_cast<float>( i ));
 
        float x = juce::jmap(logFreq, std::log(1.f), maxLogFreq, startX, startX + width);
        
        // only draw one bin per x in the GUI.
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

bool AnalyzerPathGenerator::getPath(juce::Path& path)
{
    return pathFifo.pull(path);
}
