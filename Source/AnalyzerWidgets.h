/*
  ==============================================================================

    AnalyzerWidgets.h
    Created: 30 May 2022 10:15:32am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include<JuceHeader.h>
#include "ParameterHelpers.h"
#include "EQConstants.h"

struct AnalyzerWidgetBase : juce::Component
{
    AnalyzerWidgetBase(ChainPosition cp, Channel ch);
    void displayAsSelected(bool selected);
    void paint(juce::Graphics& g) override;
    ChainPosition getChainPosition() const { return chainPosition; }
    Channel getChannel() const { return channel; }
protected:
    bool isSelected = false;
    /*
     in order to know what param you should be controlling, you need to know the chainPos and channel you're associated with.
     */
    ChainPosition chainPosition;
    Channel channel;
};


struct AnalyzerBand : AnalyzerWidgetBase
{
    AnalyzerBand(ChainPosition cp, Channel ch);
    void paint(juce::Graphics& g) override;
};

// ADD later
//struct AnalyzerQControl : AnalyzerWidgetBase
//{
//    AnalyzerQControl() : AnalyzerWidgetBase(ChainPosition::Peak1, Channel::Left) { setMouseCursor(juce::MouseCursor::LeftRightResizeCursor); }
//    void setChainPosition(ChainPosition cp) { chainPosition = cp; }
//    void setChannel(Channel ch) { channel = ch; }
//};

struct AnalyzerNode : AnalyzerWidgetBase
{
    AnalyzerNode(ChainPosition cp, Channel ch) : AnalyzerWidgetBase(cp, ch)
    {
        setSize(nodeSize, nodeSize);
    }
    static constexpr int nodeSize{8};
    juce::Rectangle<int> mouseOverBounds, qualityBounds;
    float getFrequency() const { return frequency; }
    float getGainOrSlope() const { return gainOrSlope; }
    /** returns true if the frequency was updated */
    bool updateFrequency(float f) { return update(frequency, f); }
    /** returns true if the gain/slope was updated */
    bool updateGainOrSlope(float g) { return update(gainOrSlope, g); }
private:
    float frequency = 0.f;
    float gainOrSlope = NEGATIVE_INFINITY;
    
    bool update(float& oldVal, float newVal)
    {
        if( oldVal == newVal )
            return false;
        
        oldVal = newVal;
        return true;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalyzerNode)
};


