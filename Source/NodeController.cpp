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
    
    setComponentID("CONTROLLER");
    for(uint i=0; i< 8; ++i)
    {
        auto pos = static_cast<ChainPosition>(i);
       
        nodes[i] = std::make_unique<AnalyzerNode>(pos, Channel::Left);
        nodes[i]->setComponentID(juce::String("NODE:") + "L:" + std::to_string(i));
        nodes[i]->addMouseListener(this, false);
        addChildComponent(*nodes[i]);
       
        nodes[i + 8] = std::make_unique<AnalyzerNode>(pos, Channel::Right);
        nodes[i + 8]->setComponentID(juce::String("NODE:") + "R:" + std::to_string(i));
        nodes[i + 8]->addMouseListener(this, false);
        addChildComponent(*nodes[i + 8]);
    }
    
    allParamsListener.reset( new AllParamsListener(apvts,std::bind(&NodeController::refreshWidgets,  this)));
  
}

void NodeController::resized()
{
    AnalyzerBase::resized();
    refreshNodes();
}

void NodeController::refreshWidgets()
{
    refreshNodes();
}

void NodeController::refreshNodes()
{
   
    auto bBox = fftBoundingBox.toFloat();
    bool drawRightMid = static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load()) != ChannelMode::Stereo;
    
    for(uint i=0; i< 8; ++i)
    {
        auto pos = static_cast<ChainPosition>(i);
        updateNode(*nodes[i], pos, Channel::Left, bBox);
    
        if(drawRightMid)
        {
            updateNode(*nodes[i + 8], pos, Channel::Right, bBox);
        }
    }
}




void NodeController::updateNode(AnalyzerNode& node,ChainPosition chainPos, Channel channel, juce::Rectangle<float> bBox)
{
    auto const size = AnalyzerNode::nodeSize;
    float freq;
    float gainOrSlope;
    bool bypassed;
    
    if(chainPos == ChainPosition::LowCut || chainPos == ChainPosition::HighCut)
    {
        auto cutParams = ChainHelpers::getFilterParams<HighCutLowCutParameters>(chainPos, channel, 0.0, apvts);
        freq = cutParams.frequency;
        gainOrSlope = 30 - cutParams.order * 6.f;
        bypassed = cutParams.bypassed;
    }
    else
    {
        auto params = ChainHelpers::getFilterParams<FilterParameters>(chainPos, channel, 0.0, apvts);
        freq = params.frequency;
        gainOrSlope = params.gain.getDb();
        bypassed = params.bypassed;
    }
    
    if(bypassed)
    {
        node.setVisible(false);
        return;
    }
    auto y = juce::jmap(gainOrSlope, static_cast<float>(RESPONSE_CURVE_MIN_DB), static_cast<float>(RESPONSE_CURVE_MAX_DB), bBox.getBottom(), bBox.getY());
    auto x = juce::mapFromLog10(freq, 20.f, 20000.f) * bBox.getWidth() + bBox.getX();
    
    node.setBounds(x - size / 2, y - size / 2, size, size);
    node.updateFrequency(freq);
    node.updateGainOrSlope(gainOrSlope);
    node.setVisible(true);
}



void debugMouse(juce::String type, const juce::MouseEvent &event)
{
    auto component =  event.originalComponent -> getComponentID();
    DBG(type + "from: " + component);
}

// Mouse Handling
void NodeController::mouseMove(const juce::MouseEvent &event)
{
    
}


void NodeController::mouseEnter(const juce::MouseEvent &event)
{
    debugMouse("Enter", event);
}

void NodeController::mouseExit(const juce::MouseEvent &event)
{
    debugMouse("Exit", event);
}

void NodeController::mouseDown(const juce::MouseEvent &event)
{
    debugMouse("Down", event);
}

void NodeController::mouseDrag(const juce::MouseEvent &event)
{
    debugMouse("Drag", event);
}

void NodeController::mouseUp(const juce::MouseEvent &event)
{
    debugMouse("Mouse Up", event);
}

void NodeController::mouseDoubleClick(const juce::MouseEvent &event)
{
    debugMouse("DoubleClick", event);
}



