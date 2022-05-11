/*
  ==============================================================================

    DualBypassButton.h
    Created: 9 May 2022 11:44:33am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BypassButton.h"
#include "ParamListener.h"
#include "ParameterHelpers.h"

//==============================================================================
/*
*/



class DualBypassButton  : public juce::Component
{
public:
    DualBypassButton(int filterNum, juce::AudioProcessorValueTreeState& apvts);
    ~DualBypassButton() override = default;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
   

private:
    bool isShowingAsOn(Channel channel);
    void refreshButtons(ChannelMode mode);
    
    bool isPaired {false};
    
    struct PathPoints
    {
        float xStart, xLeft, xMid, xRight, xEnd;
        float yTop, yMid, yBottom;
    };
    
    PathPoints computePathPoints(juce::Rectangle<float>& bounds);
    void drawCut(PathPoints, juce::Graphics& g, juce::AffineTransform& transform, juce::Colour, juce::Colour);
    void drawShelf(PathPoints, juce::Graphics& g, juce::AffineTransform& transform, juce::Colour, juce::Colour);
    void drawPeak(PathPoints, juce::Graphics& g, juce::Colour leftColor, juce::Colour rightColor);
    
    // colors for drawing the lines
    const juce::Colour onColor = juce::Colours::darkblue;
    const juce::Colour offColor = juce::Colours::lightblue;
    
    int filterNum;
    BypassButton leftMidBypass;
    BypassButton rightSideBypass;
    
    //juce::ToggleButton leftMidBypass;
    //juce::ToggleButton rightSideBypass;
    
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> leftMidAttachment;
    std::unique_ptr<ButtonAttachment> rightSideAttachment;
    
    std::unique_ptr<ParamListener> modeListener;
    
    juce::AudioProcessorValueTreeState& apvts;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualBypassButton)
};
