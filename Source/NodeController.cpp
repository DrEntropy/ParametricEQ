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

float bandWidthFromQ(float Q)
{
    if(Q <= 0)
    {
        return 20000;
    }
    
    // bandwidth in octaves, assume sufficiently high sample rate.
    // https://www.w3.org/TR/audio-eq-cookbook/
    
    return std::asinh(1 / (2 * Q)) * 2 / std::log(2.f);
}


std::tuple<float, float, float, bool> getParameterTuple(ChainPosition chainPos, Channel channel, juce::AudioProcessorValueTreeState& apvts)
{
    float freq, gainOrSlope, Q;
    bool bypassed;
    
    if(chainPos == ChainPosition::LowCut || chainPos == ChainPosition::HighCut)
    {
        auto cutParams = ChainHelpers::getFilterParams<HighCutLowCutParameters>(chainPos, channel, 0.0, apvts);
        freq = cutParams.frequency;
        gainOrSlope = 30 - cutParams.order * 6.f;
        Q = cutParams.quality;
        bypassed = cutParams.bypassed;
    }
    else
    {
        auto params = ChainHelpers::getFilterParams<FilterParameters>(chainPos, channel, 0.0, apvts);
        freq = params.frequency;
        gainOrSlope = params.gain.getDb();
        Q = params.quality;
        bypassed = params.bypassed;
    }
    return {freq, gainOrSlope, Q, bypassed};
}

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
    

    
    auto addAttachments = [&](size_t i, ChainPosition pos, Channel ch)
    {
        freqAttachements[i] = std::make_unique<ParameterAttachment>(*getFrequencyParam(apvts, ch, pos), nullptr);
        qAttachements[i] = std::make_unique<ParameterAttachment>(*getQParam(apvts, ch, pos), nullptr);
        gainOrSlopeAttachements[i] = std::make_unique<ParameterAttachment>(*getGainOrSlopeParam(apvts, ch, pos), nullptr);
    };
    
    setComponentID("CONTROLLER");
    for(uint i=0; i< 8; ++i)
    {
        auto pos = static_cast<ChainPosition>(i);
        
        addWidget(i, bands, pos, Channel::Left);
        addWidget(i + 8, bands, pos, Channel::Right);
       
        addWidget(i, nodes, pos, Channel::Left);
        addWidget(i + 8, nodes, pos, Channel::Right);
        
        addAttachments(i, pos, Channel::Left);
        addAttachments(i + 8, pos, Channel::Right);
    }
    
    allParamsListener.reset( new AllParamsListener(apvts,std::bind(&NodeController::refreshWidgets,  this)));

     
  
}

void NodeController::resized()
{
    AnalyzerBase::resized();
    refreshWidgets();
    
    
    constrainer.boundsLimit = [&]()
    {
        auto nodeLimit = fftBoundingBox;
        auto controlRange = getGainOrSlopeParam(apvts, Channel::Left, ChainPosition::PeakFilter1)->getNormalisableRange().getRange();
        auto pixelsPerDB = fftBoundingBox.getHeight() / (RESPONSE_CURVE_MAX_DB - RESPONSE_CURVE_MIN_DB);
        nodeLimit.setTop(fftBoundingBox.getY() + pixelsPerDB * (RESPONSE_CURVE_MAX_DB - controlRange.getEnd()));
        nodeLimit.setBottom(fftBoundingBox.getBottom() - pixelsPerDB * (controlRange.getStart() - RESPONSE_CURVE_MIN_DB));
        return nodeLimit;
    }();
    
    hConstrainer.boundsLimit = constrainer.boundsLimit;
}

void NodeController::refreshWidgets()
{
    auto bBox = fftBoundingBox.toFloat();
    bool drawRightMid = static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load()) != ChannelMode::Stereo;
    
    for(uint i=0; i< 8; ++i)
    {
        updateNode(*nodes[i], bBox);
        updateBand(*bands[i], bBox);

        if(drawRightMid)
        {
            updateNode(*nodes[i + 8], bBox);
            updateBand(*bands[i + 8], bBox);
        }
    }
}




 

void NodeController::updateNode(AnalyzerNode& node, juce::Rectangle<float> bBox)
{
    auto const size = AnalyzerNode::nodeSize;
    
    auto chainPos = node.getChainPosition();
    auto channel = node.getChannel();
    
    auto [freq, gainOrSlope, Q, bypassed] = getParameterTuple(chainPos, channel, apvts);
    
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

void NodeController::updateBand(AnalyzerBand& band, juce::Rectangle<float> bBox)
{
    auto chainPos = band.getChainPosition();
    auto channel = band.getChannel();
    
    auto [freq, gainOrSlope, Q, bypassed] = getParameterTuple(chainPos, channel, apvts);
    
    if(bypassed)
    {
        band.setVisible(false);
        return;
    }
    
    auto centerX = juce::mapFromLog10(freq, 20.f, 20000.f) * bBox.getWidth() + bBox.getX();
    
    auto bounds = bBox;
    
    
    auto BW = bandWidthFromQ(Q);
    
    double widthOctaves = std::log2(20000.0 / 20.0);
    bounds.setWidth(bBox.getWidth() * BW / widthOctaves);
    
    bounds.setCentre(centerX, bBox.getCentreY());
    
    band.setBounds(bounds.toNearestInt());
    band.setVisible(true);
}


// Mouse Handling
void NodeController::mouseMove(const juce::MouseEvent &event)
{
    
}



// TODO: these could be handled in the components themselves
void NodeController::mouseEnter(const juce::MouseEvent &event)
{
    if(auto widget = dynamic_cast<AnalyzerWidgetBase*>(event.originalComponent))
    {
        widget->displayAsSelected(true);
     }
    debugMouse("Enter", event);
}


void NodeController::mouseExit(const juce::MouseEvent &event)
{
    if(auto widget = dynamic_cast<AnalyzerWidgetBase*>(event.originalComponent))
    {
        widget->displayAsSelected(false);
    }
   
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
            
        case WidgetVariant::Band:
        {
            break;
        }
        
        case WidgetVariant::QControl:
        {
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
        
        case WidgetVariant::QControl:
        {
            break;
        }
            
        default:
            ;
            
    }
   // debugMouse("Drag", event);
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
