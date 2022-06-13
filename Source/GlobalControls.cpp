/*
  ==============================================================================

    GlobalControls.cpp
    Created: 12 Jun 2022 10:21:44am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "GlobalControls.h"

//==============================================================================
GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apv, NodeController& nodeController):apvts(apv), nodeControl(nodeController),
                               inGain(apv, GlobalParameters::inTrimName),
                               outGain(apv, GlobalParameters::outTrimName),
                               processingMode(apv, GlobalParameters::processingModeName), analyzerControls(apv)
{
    setLookAndFeel(&lookAndFeel);
    
    addAndMakeVisible(analyzerControls);
    addAndMakeVisible(outGain);
    addAndMakeVisible(inGain);
    addAndMakeVisible(processingMode);
    addAndMakeVisible(resetAllBands);
    
    resetAllBands.onClick = [&]()
    {
       nodeControl.resetAllParameters();
    };
}

GlobalControls::~GlobalControls()
{
    setLookAndFeel(nullptr);
}

 

void GlobalControls::resized()
{
    auto bounds = getLocalBounds();
    //   make room for square bounded controls
    auto controlWidth = bounds.getHeight();
    
    //controls are square:
    auto inTrimBounds = bounds.removeFromLeft(controlWidth);
    auto outTrimBounds = bounds.removeFromRight(controlWidth);
    bounds.removeFromLeft(controlWidth / 2); //space between in trim and proc mode
    auto procModeBounds = bounds.removeFromLeft(controlWidth);
    
    // analyzer control has 4 buttons, but make it 4.5 = 9/2 for abit extra room.
    auto analyzerControlBounds =  bounds.removeFromLeft(bounds.getHeight() * 9  / 2);
    inGain.setBounds(inTrimBounds);
    outGain.setBounds(outTrimBounds);
    processingMode.setBounds(procModeBounds);
    analyzerControls.setBounds(analyzerControlBounds);
    
    auto resetBounds  = bounds.expanded(-20, -20);
    resetAllBands.setBounds(resetBounds);
    
  
}
