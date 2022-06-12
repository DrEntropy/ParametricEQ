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

//==============================================================================
/*
*/
class GlobalControls  : public juce::Component
{
public:
    GlobalControls(juce::AudioProcessorValueTreeState& apv);
    ~GlobalControls() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    juce::AudioProcessorValueTreeState& apvts;
    
    AnalyzerControls analyzerControls;
    
 
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalControls)
};
