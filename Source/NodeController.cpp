/*
  ==============================================================================

    NodeController.cpp
    Created: 30 May 2022 10:10:21am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "NodeController.h"
#include "ChainHelpers.h"
#include <variant>


struct WidgetVariant
{
    std::variant<std::monostate, NodeController*, AnalyzerNode*, AnalyzerQControl*, AnalyzerBand*>  component;
    enum Indices
    {
        Invalid,
        Controller,
        Node,
        QControl,
        Band
    };
};
 

WidgetVariant getEventsComponent(const juce::MouseEvent &event)
{
    WidgetVariant widgetVar;
    auto componentPtr =  event.originalComponent;
    if(auto nodeController = dynamic_cast<NodeController*>(componentPtr))
    {
        widgetVar.component = nodeController;
    }
    else if(auto analyzerNode = dynamic_cast<AnalyzerNode *>(componentPtr))
    {
        widgetVar.component = analyzerNode;
    }
    else if(auto analyzerQ = dynamic_cast<AnalyzerQControl *>(componentPtr))
    {
        widgetVar.component = analyzerQ;
    }
    else if(auto band = dynamic_cast<AnalyzerBand *>(componentPtr))
    {
        widgetVar.component = band;
    }
    else
    {
        widgetVar.component = std::monostate();
        jassertfalse;  // Should not happen
    }
    return widgetVar;
}

struct ChannelVisitor
{
    Channel operator()(AnalyzerNode* node) const
    {
        return node->getChannel();
    }
    Channel operator()(AnalyzerBand* band) const
    {
        return band->getChannel();
    }
    Channel operator()(AnalyzerQControl* qc) const
    {
        return qc->getChannel();
    }
    Channel operator()(NodeController*) const
    {
        return Channel::Left;
    }
    Channel operator()(std::monostate) const
    {
        jassertfalse;
        return Channel::Left;
    }
};

void NodeController::debugMouse(juce::String type, const juce::MouseEvent &event)
{
    auto componentID =  event.originalComponent -> getComponentID();
    auto widgetVar = getEventsComponent(event);
    juce::String channelLabel{"None"};
    
    if(widgetVar.component.index() != WidgetVariant::Invalid  && widgetVar.component.index() != WidgetVariant::Invalid)
    {
        auto channel = std::visit(ChannelVisitor{}, widgetVar.component);
        channelLabel = (channel == Channel::Left ? "Left" : "Right");
    }
    
    int x = 0;
    int y = 0;
    if(widgetVar.component.index() != WidgetVariant::Controller)
    {
        x = event.originalComponent->getX();
        y = event.originalComponent->getY();
    }
    
     
    
    DBG(type + "from: " + componentID + " ch:" + channelLabel + " freq:" +
        std::to_string(frequencyFromX(x + event.x))  + " gain:" + std::to_string(gainFromY(y + event.y)) );
    
    if(widgetVar.component.index() == WidgetVariant::Node)
        DBG("Node frequency:" + std::to_string(std::get<AnalyzerNode *>(widgetVar.component)->getFrequency()));
}

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
        
        
        freqAttachements[i] = std::make_unique<ParameterAttachment>(*getFrequencyParam(apvts, Channel::Left, pos), nullptr);
        qAttachements[i] = std::make_unique<ParameterAttachment>(*getQParam(apvts, Channel::Left, pos), nullptr);
        gainOrSlopeAttachements[i] = std::make_unique<ParameterAttachment>(*getGainOrSlopeParam(apvts, Channel::Left, pos), nullptr);
        
        freqAttachements[i + 8] = std::make_unique<ParameterAttachment>(*getFrequencyParam(apvts, Channel::Right, pos), nullptr);
        qAttachements[i + 8] = std::make_unique<ParameterAttachment>(*getQParam(apvts, Channel::Right, pos), nullptr);
        gainOrSlopeAttachements[i + 8] = std::make_unique<ParameterAttachment>(*getGainOrSlopeParam(apvts, Channel::Right, pos), nullptr);
    }
    
    allParamsListener.reset( new AllParamsListener(apvts,std::bind(&NodeController::refreshWidgets,  this)));

     
  
}

void NodeController::resized()
{
    AnalyzerBase::resized();
    refreshNodes();
    
    auto nodeLimit = fftBoundingBox;
    auto controlRange = getGainOrSlopeParam(apvts, Channel::Left, ChainPosition::PeakFilter1)->getNormalisableRange().getRange();
    auto pixelsPerDB = fftBoundingBox.getHeight() / (RESPONSE_CURVE_MAX_DB - RESPONSE_CURVE_MIN_DB);
    nodeLimit.setTop(fftBoundingBox.getY() + pixelsPerDB * (RESPONSE_CURVE_MAX_DB - controlRange.getEnd()));
    nodeLimit.setBottom(fftBoundingBox.getBottom() - pixelsPerDB * (controlRange.getStart() - RESPONSE_CURVE_MIN_DB));
    constrainer.boundsLimit = nodeLimit;
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
    
    auto widgetVar = getEventsComponent(event);
    // use a visitor here?
    switch (widgetVar.component.index())
    {
        case WidgetVariant::Node:
        {
            auto node = std::get<AnalyzerNode*>(widgetVar.component);
            dragger.startDraggingComponent(node, event);
            getAttachmentForNode(freqAttachements, node).beginGesture();
            getAttachmentForNode(gainOrSlopeAttachements, node).beginGesture();
            break;
        }
        default:
            ;
            
    }
    debugMouse("Down", event);
}

void NodeController::mouseDrag(const juce::MouseEvent &event)
{
    
    auto widgetVar = getEventsComponent(event);
    switch (widgetVar.component.index())
    {
        case WidgetVariant::Node:
        {
            auto node = std::get<AnalyzerNode*>(widgetVar.component);
            dragger.dragComponent(node, event, &constrainer);
            
            node->updateFrequency(frequencyFromX(node->getX() + node->getWidth() / 2.0));
            
            float gainOrSlopeUnnorm;
            auto& gainOrSlopeAttach = getAttachmentForNode(gainOrSlopeAttachements, node);
            
            if(node->getChainPosition() == ChainPosition::LowCut || node->getChainPosition() == ChainPosition::HighCut)
            {
                auto slope = slopeFromY(node->getY() + node->getHeight() / 2.0);
                node->updateGainOrSlope(slope);
                gainOrSlopeUnnorm = (slope-6.f)/6.f;
            }
            else
            {
                gainOrSlopeUnnorm = gainFromY(node->getY() + node->getHeight() / 2.0);
                node->updateGainOrSlope(gainOrSlopeUnnorm);
            }
            gainOrSlopeAttach.setValueAsPartOfGesture(gainOrSlopeUnnorm);
            
            getAttachmentForNode(freqAttachements, node).setValueAsPartOfGesture(node->getFrequency());
            
            break;
        }
        
        case WidgetVariant::Band:
        {
            break;
        }
        default:
            ;
            
    }
    debugMouse("Drag", event);
}

void NodeController::mouseUp(const juce::MouseEvent &event)
{
    auto widgetVar = getEventsComponent(event);
    switch (widgetVar.component.index())
    {
        case WidgetVariant::Node:
        {
            auto node = std::get<AnalyzerNode*>(widgetVar.component);
            getAttachmentForNode(freqAttachements, node).endGesture();
            getAttachmentForNode(gainOrSlopeAttachements, node).endGesture();
            break;
        }
        default:
            ;
            
    }
    debugMouse("Mouse Up", event);
}

void NodeController::mouseDoubleClick(const juce::MouseEvent &event)
{
    debugMouse("DoubleClick", event);
}

float NodeController::frequencyFromX(float x)
{
    auto bBox = fftBoundingBox.toFloat();
    return juce::mapToLog10((x - bBox.getX()) /  bBox.getWidth(), 20.f, 20000.f);
}

float NodeController::gainFromY(float y)
{
    auto bBox = fftBoundingBox.toFloat();
    return juce::jmap(y, bBox.getBottom(), bBox.getY(), static_cast<float>(RESPONSE_CURVE_MIN_DB), static_cast<float>(RESPONSE_CURVE_MAX_DB ));
}

float NodeController::slopeFromY(float y)
{
    auto bBox = fftBoundingBox.toFloat();
    return juce::jmap(y, bBox.getBottom(), bBox.getY(), 30.f - static_cast<float>(RESPONSE_CURVE_MIN_DB), 30.f - static_cast<float>(RESPONSE_CURVE_MAX_DB));
}


ParameterAttachment& NodeController::getAttachmentForNode(std::array<std::unique_ptr<ParameterAttachment>, 16>& attachments, AnalyzerNode* node)
{
    Channel ch = node->getChannel();
    ChainPosition cp = node->getChainPosition();
    size_t filterNum = static_cast<size_t>(cp);
    
    if(ch == Channel::Left)
        return *attachments[filterNum];
 
    return *attachments[filterNum + 8];
}
