/*
  ==============================================================================

    AnalyzerBase.h
    Created: 17 May 2022 9:21:12pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
struct AnalyzerBase : juce::Component
{
    juce::Rectangle<int> getBoundsForRendering() const
    {
        return getLocalBounds().reduced(0, 3 * getTextHeight()/4);
    }
    
    juce::Rectangle<int> getBoundsForFFT()
    {
        auto bounds = getBoundsForRendering();
        auto w = getTextWidthScaled();
        return bounds.withTrimmedLeft(w).withTrimmedRight(w);
    }
    
    inline int getTextHeight() const { return 16; }
    inline int getTextWidth() const { return 16; }
    void resized() override
    {
        fftBoundingBox = getBoundsForFFT();
    }
protected:
    juce::Rectangle<int> fftBoundingBox;
    inline float getTextWidthScaled() const {return getTextWidth() * 3 / 2;}
};
