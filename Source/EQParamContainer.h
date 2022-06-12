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
#include "NodeController.h"
//==============================================================================
/*
*/
class EQParamContainer  : public juce::Component, public NodeController::Listener
{
public:
    EQParamContainer(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        
        for(auto& widget : widgets)
           addAndMakeVisible(widget);

    }

    ~EQParamContainer() override = default;
 
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto width = bounds.getWidth();
        
        for(auto& widget : widgets)
            widget.setBounds(bounds.removeFromLeft(width / 8));
    }

    // currently no difference in selected vs moused over here currently.
    void bandMousedOver(ChainPosition cp, Channel ch) override
    {
        clearSelection();
        widgets.at(static_cast<int>(cp)).bandSelected(ch);
    }
    
    void bandSelected(ChainPosition cp, Channel ch) override
    {
        clearSelection();
        widgets.at(static_cast<int>(cp)).bandSelected(ch);
    }
    
    void clearSelection() override
    {
        for(auto& widget:widgets)
            widget.bandCleared();
    }
    
private:
    
    juce::AudioProcessorValueTreeState& apvts;
    
    std::array<EQParamWidget, 8> widgets
    {{
        {apvts, ChainPosition::LowCut, true},
        {apvts, ChainPosition::LowShelf, false},
        {apvts, ChainPosition::PeakFilter1, false},
        {apvts, ChainPosition::PeakFilter2, false},
        {apvts, ChainPosition::PeakFilter3, false},
        {apvts, ChainPosition::PeakFilter4, false},
        {apvts, ChainPosition::HighShelf, false},
        {apvts, ChainPosition::HighCut, true}
    }};
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamContainer)
};
