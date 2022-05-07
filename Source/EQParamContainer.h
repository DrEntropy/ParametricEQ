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
    EQParamContainer(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        addAndMakeVisible(lowCut);
        addAndMakeVisible(param1);
        addAndMakeVisible(param2);
        addAndMakeVisible(param3);
        addAndMakeVisible(param4);
        addAndMakeVisible(param5);
        addAndMakeVisible(param6);
        addAndMakeVisible(highCut);

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
        param1.setBounds(bounds.removeFromLeft(width/8));
        param2.setBounds(bounds.removeFromLeft(width/8));
        param3.setBounds(bounds.removeFromLeft(width/8));
        param4.setBounds(bounds.removeFromLeft(width/8));
        param5.setBounds(bounds.removeFromLeft(width/8));
        param6.setBounds(bounds.removeFromLeft(width/8));
        highCut.setBounds(bounds);
        
    }

private:
    
    juce::AudioProcessorValueTreeState& apvts;
    
    EQParamWidget lowCut {apvts, 0, true};
    EQParamWidget param1 {apvts, 1, false};
    EQParamWidget param2 {apvts, 2, false};
    EQParamWidget param3 {apvts, 3, false};
    EQParamWidget param4 {apvts, 4, false};
    EQParamWidget param5 {apvts, 5, false};
    EQParamWidget param6 {apvts, 6, false};
    EQParamWidget highCut {apvts, 7, true};
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamContainer)
};
