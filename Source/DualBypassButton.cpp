/*
  ==============================================================================

    DualBypassButton.cpp
    Created: 9 May 2022 11:44:33am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DualBypassButton.h"
#include "ParameterHelpers.h"

//==============================================================================
DualBypassButton::DualBypassButton(ChainPosition cp, juce::AudioProcessorValueTreeState& apvts):chainPos(cp), apvts(apvts)
{
    addAndMakeVisible(leftMidBypass);
    addAndMakeVisible(rightSideBypass);
    
    rightSideBypass.onLeft = false;
    leftMidBypass.onLeft = true;
    rightSideBypass.isPaired = true;  //never on its own.
 
    leftMidAttachment.reset(new ButtonAttachment(apvts, createBypassParamString(Channel::Left, chainPos), leftMidBypass));
    rightSideAttachment.reset(new ButtonAttachment(apvts, createBypassParamString(Channel::Right, chainPos), rightSideBypass));
    
    leftMidBypass.onClick = [this]() { this->repaint(); };
    rightSideBypass.onClick = [this]() { this->repaint(); };
    
    auto safePtr = juce::Component::SafePointer<DualBypassButton>(this);
    modeListener.reset(new ParamListener(apvts.getParameter("Processing Mode"),
                                        [safePtr](float v)
                                         {
                                          if(auto* comp = safePtr.getComponent() )
                                              comp->refreshButtons(static_cast<ChannelMode>(v));
                                         }));
}



void DualBypassButton::paintOverChildren(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(offColor);
    g.drawRect(bounds, 2);
    
    auto midX = bounds.getX() + bounds.getWidth() / 2.f;
    auto dashedLine = juce::Line<float>(midX, bounds.getY(), midX, bounds.getY() + bounds.getHeight());
    float dashPattern[2] {5.f, 5.f};
    
    if(isPaired)
        g.drawDashedLine(dashedLine, dashPattern, 2);
    
    
    auto pp = computePathPoints(bounds);
    
    auto unflipped = juce::AffineTransform();
    auto flipped = unflipped.scale(-1.f, 1.f, pp.xMid, pp.yMid);
    
    auto leftColor =  isShowingAsOn(Channel::Left) ? onColor : offColor;
    auto rightColor = (isShowingAsOn(Channel::Left) && ! isPaired) || (isShowingAsOn(Channel::Right) && isPaired) ? onColor : offColor;

    
    switch(chainPos)
    {
        case ChainPosition::LowCut:
            drawCut(pp, g, unflipped, leftColor, rightColor);
            break;
            
        case ChainPosition::LowShelf:
            drawShelf(pp, g, unflipped, leftColor, rightColor);
            break;
            
        case ChainPosition::HighShelf:
            drawShelf(pp, g, flipped,  rightColor, leftColor);
            break;
            
        case ChainPosition::HighCut:
            drawCut(pp, g, flipped, rightColor, leftColor);
            break;
            
        default:
            drawPeak(pp, g, leftColor, rightColor);
            break;
    }
}


DualBypassButton::PathPoints DualBypassButton::computePathPoints(juce::Rectangle<float>& bounds)
{
    PathPoints pathPoints;
    
    auto x = bounds.getX();
    auto y = bounds.getY();
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();
    
    pathPoints.xStart = x + w / 8.f;
    pathPoints.xLeft  = x + 3.f * w / 8.f;
    pathPoints.xMid =  x + w / 2.f;
    pathPoints.xRight = x + 5.f * w / 8.f;
    pathPoints.xEnd = x + 7 * w / 8.f;
    pathPoints.yTop = y + h / 4.f;
    pathPoints.yMid = y + h / 2.f;
    pathPoints.yBottom = y + 3.f * h / 4.f;

    return pathPoints;
}

void DualBypassButton::drawCut(PathPoints pp, juce::Graphics& g, juce::AffineTransform& transform, juce::Colour leftColor, juce::Colour rightColor)
{
    auto leftpath = juce::Path();
    auto rightpath = juce::Path();
    
    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    
    leftpath.startNewSubPath(pp.xStart, pp.yBottom);
    leftpath.lineTo(pp.xLeft, pp.yTop);
    leftpath.lineTo(pp.xMid, pp.yTop);
    
    g.setColour(leftColor);
    g.strokePath(leftpath, pst, transform);
    
    rightpath.startNewSubPath(pp.xMid, pp.yTop);
    rightpath.lineTo(pp.xEnd, pp.yTop);

    g.setColour(rightColor);
    g.strokePath(rightpath, pst, transform);
}
 

void DualBypassButton::drawShelf(PathPoints pp, juce::Graphics& g, juce::AffineTransform& transform, juce::Colour leftColor, juce::Colour rightColor)
{
    auto leftpath = juce::Path();
    auto rightpath = juce::Path();
    
    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    
    leftpath.startNewSubPath(pp.xStart, pp.yBottom);
    leftpath.lineTo(pp.xLeft, pp.yBottom);
    leftpath.lineTo(pp.xMid, pp.yMid);
    leftpath.startNewSubPath(pp.xStart, pp.yTop);
    leftpath.lineTo(pp.xLeft, pp.yTop);
    leftpath.lineTo(pp.xMid, pp.yMid);
    
    g.setColour(leftColor);
    g.strokePath(leftpath, pst, transform);
    
    rightpath.startNewSubPath(pp.xMid, pp.yMid);
    rightpath.lineTo(pp.xEnd, pp.yMid);

    g.setColour(rightColor);
    g.strokePath(rightpath, pst, transform);
}

void DualBypassButton::drawPeak(PathPoints pp, juce::Graphics& g, juce::Colour leftColor, juce::Colour rightColor)
{
    auto leftpath = juce::Path();
    auto rightpath = juce::Path();
    
    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    
    leftpath.startNewSubPath(pp.xStart, pp.yMid);
    leftpath.lineTo(pp.xLeft, pp.yMid);
    leftpath.lineTo(pp.xMid, pp.yTop);
    leftpath.startNewSubPath(pp.xLeft, pp.yMid);
    leftpath.lineTo(pp.xMid, pp.yBottom);
    
    g.setColour(leftColor);
    g.strokePath(leftpath, pst);
    
    auto flipped = juce::AffineTransform().scale(-1.f, 1.f, pp.xMid, pp.yMid);

    g.setColour(rightColor);
    g.strokePath(leftpath, pst, flipped);
}


void DualBypassButton::resized()
{
    refreshButtons(static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load()));
}


void DualBypassButton::refreshButtons(ChannelMode mode)
{
    auto bounds = getLocalBounds();
    
    if(mode == ChannelMode::Stereo)
    {
        isPaired = false;
        leftMidBypass.isPaired = false;
        rightSideBypass.setVisible(false);
        leftMidBypass.setBounds(bounds);
    }
    else
    {
        isPaired = true;
        leftMidBypass.isPaired = true;
        rightSideBypass.setVisible(true);
        leftMidBypass.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
        rightSideBypass.setBounds(bounds);
    }
}


bool DualBypassButton::isShowingAsOn(Channel channel)
{
    if(channel == Channel::Left)
        return leftMidBypass.isShowingAsOn();
    
    return rightSideBypass.isShowingAsOn();
}
