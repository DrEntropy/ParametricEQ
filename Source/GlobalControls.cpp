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
    // square controls
    auto controlWidth = bounds.getHeight();
    
    auto inTrimBounds = bounds.removeFromLeft(controlWidth);
    auto outTrimBounds = bounds.removeFromRight(controlWidth);
    
    //processing mode control is square, but leave some room
    modeBox.setBounds(bounds.withWidth(static_cast<int>(processingModeAspectRatio * controlWidth)));
    auto procModeBounds = bounds.removeFromLeft(controlWidth);
    bounds.removeFromLeft(static_cast<int>((processingModeAspectRatio - 1.0f) * controlWidth)); // remove rest of rectangle
    
    auto analyzerControlBounds =  bounds.removeFromLeft(static_cast<int>(bounds.getHeight() * analyzerControlAspectRatio));
    inGain.setBounds(inTrimBounds);
    inGainBox.setBounds(inTrimBounds);
    
    outGain.setBounds(outTrimBounds);
    outGainBox.setBounds(outTrimBounds);
    
    processingMode.setBounds(procModeBounds);
    
    analyzerControls.setBounds(analyzerControlBounds);
    
    resetBox.setBounds(bounds);
    auto resetButtonBounds  = bounds.expanded(-resetButtonHMargin, -resetButtonVMargin);
    resetAllBands.setBounds(resetButtonBounds);
}
