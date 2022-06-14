/*
  ==============================================================================

    GlobalControls.h
    Created: 12 Jun 2022 10:21:44am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AnalyzerControls.h"
#include "BottomControl.h"
#include "GlobalParameters.h"
#include "NodeController.h"

//==============================================================================
/*
*/



class GlobalControls  : public juce::Component
{
public:
    GlobalControls(juce::AudioProcessorValueTreeState& apv, NodeController& nodeController);
    ~GlobalControls() override;
 
    void resized() override;

private:
     
    
    juce::AudioProcessorValueTreeState& apvts;
    NodeController& nodeControl;
    
    BottomControl<RotarySlider> inGain;
    BottomControl<RotarySlider> outGain;
    BottomControl<SwitchSlider> processingMode;
    
    BoundaryBox inGainBox,  outGainBox, modeBox, resetBox;
    
    AnalyzerControls analyzerControls;
    
    juce::TextButton resetAllBands{"Reset All Bands"};
    
    BottomLookAndFeel lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalControls)
};
