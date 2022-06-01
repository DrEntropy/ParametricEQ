/*
  ==============================================================================

    NodeController.h
    Created: 30 May 2022 10:10:21am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include "AnalyzerBase.h"
#include "AnalyzerWidgets.h"
#include "AllParamsListener.h"

struct NodeController : AnalyzerBase
{
    NodeController(juce::AudioProcessorValueTreeState&);
    
    
    void resized() override;
    
private:
    
    void refreshNodes();
    std::array<std::unique_ptr<AnalyzerNode> , 16> nodes; //first 8 are left/mid, second  8 are right side.
    std::unique_ptr<AllParamsListener> allParamsListener;
    
    juce::AudioProcessorValueTreeState& apvts;
};

