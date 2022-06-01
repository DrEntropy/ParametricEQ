/*
  ==============================================================================

    NodeController.cpp
    Created: 30 May 2022 10:10:21am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "NodeController.h"
#include "ChainHelpers.h"


NodeController::NodeController(juce::AudioProcessorValueTreeState& apvts) : apvts{apvts}
{
   for(uint i=0; i< 8; ++i)
   {
       auto pos = static_cast<ChainPosition>(i);
       nodes[i] = std::make_unique<AnalyzerNode>(pos, Channel::Left);
       addAndMakeVisible(*nodes[i]);
       nodes[i+8] = std::make_unique<AnalyzerNode>(pos, Channel::Right);
   }
}

void NodeController::resized()
{
    AnalyzerBase::resized();
    refreshNodes();
}


void NodeController::refreshNodes()
{
    auto size = AnalyzerNode::nodeSize;
    auto bBox = fftBoundingBox.toFloat();
    
    // inner filters
    for(uint i=1; i< 7; ++i)
    {
        auto pos = static_cast<ChainPosition>(i);
        auto params = ChainHelpers::getFilterParams<FilterParameters>(pos, Channel::Left, 0.0, apvts);
        auto y = juce::jmap(params.gain.getDb(), static_cast<float>(RESPONSE_CURVE_MIN_DB), static_cast<float>(RESPONSE_CURVE_MAX_DB), bBox.getBottom(), bBox.getY());
        auto x = juce::mapFromLog10(params.frequency, 20.f, 20000.f) * bBox.getWidth() + bBox.getX();
        nodes[i]->setBounds(x, y, size, size);
        //nodes[i+8]->setBounds(<#int x#>, <#int y#>, size, size)
    }
    
    //cut filters , factor this later.
   // auto lowCutParamsL = ChainHelpers::getFilterParams<HighCutLowCutParameters>(ChainPosition::LowCut, Channel::Left, 0.0, apvts);
   // auto lowCutParamsR = ChainHelpers::getFilterParams<HighCutLowCutParameters>(ChainPosition::LowCut, Channel::Right, 0.0, apvts);
}
