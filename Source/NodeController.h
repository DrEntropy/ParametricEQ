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
#include "ParameterAttachment.h"

struct NodeController : AnalyzerBase 
{
    NodeController(juce::AudioProcessorValueTreeState&);
    void resized() override;
    
    // Listeners
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;
    
private:
    
    void refreshWidgets();
    void refreshNodes();
    void updateNode(AnalyzerNode& node, ChainPosition chainPos, Channel channel, juce::Rectangle<float> bBox);
    void debugMouse(juce::String type, const juce::MouseEvent &event);
    
    float frequencyFromX(float x);
    float gainFromY(float y);
    float slopeFromY(float y);
    
    bool adjustingNode{false};
    float prevFreq, prevGainOrSlope;
    
    std::array<std::unique_ptr<AnalyzerNode> , 16> nodes; //first 8 are left/mid, second  8 are right side.
    std::array<std::unique_ptr<ParameterAttachment>, 16> freqAttachements;
    std::array<std::unique_ptr<ParameterAttachment>, 16> qAttachements;
    std::array<std::unique_ptr<ParameterAttachment>, 16> gainOrSlopeAttachements;
    
    std::unique_ptr<AllParamsListener> allParamsListener;
    
    ParameterAttachment& getAttachmentForNode(std::array<std::unique_ptr<ParameterAttachment>, 16>& attachments, AnalyzerNode* node);
 
    
    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer constrainer{};
    
    juce::AudioProcessorValueTreeState& apvts;
};

