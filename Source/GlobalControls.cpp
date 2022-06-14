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
    
    addAndMakeVisible(inGainBox);
    addAndMakeVisible(outGainBox);
    addAndMakeVisible(modeBox);
    addAndMakeVisible(resetBox);
    
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
     
    auto controlWidth = bounds.getHeight();
    
    // trim controls are square:
    auto inTrimBounds = bounds.removeFromLeft(controlWidth);
    auto outTrimBounds = bounds.removeFromRight(controlWidth);
    
    modeBox.setBounds(bounds.withWidth(3 * controlWidth / 2));
    auto procModeBounds = bounds.removeFromLeft(controlWidth);
    bounds.removeFromLeft(controlWidth / 2); //space between in trim and proc mode
    
    // analyzer control has 4 buttons, but make it 4.5 = 9/2 for abit extra room.
    auto analyzerControlBounds =  bounds.removeFromLeft(bounds.getHeight() * 9  / 2);
    inGain.setBounds(inTrimBounds);
    inGainBox.setBounds(inTrimBounds);
    
    outGain.setBounds(outTrimBounds);
    outGainBox.setBounds(outTrimBounds);
    
    processingMode.setBounds(procModeBounds);
     
    
    analyzerControls.setBounds(analyzerControlBounds);
    
    resetBox.setBounds(bounds);
    
    auto resetBounds  = bounds.expanded(-20, -30);
    resetAllBands.setBounds(resetBounds);
    
  
}
