/*
  ==============================================================================

    EQParamContainer.h
    Created: 5 May 2022 1:22:04pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EQParamWidget.h"

//==============================================================================
/*
*/
class EQParamContainer  : public juce::Component
{
public:
    EQParamContainer()
    {
        addAndMakeVisible(lowCut);

    }

    ~EQParamContainer() override
    {
    }

    void paint (juce::Graphics& g) override
    {
 
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
 
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto width = bounds.getWidth();
        lowCut.setBounds(bounds.removeFromLeft(width/8));
    }

private:
    
    EQParamWidget lowCut {0, true};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamContainer)
};
