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
 
size_t getWidgetIndex(ChainPosition cp, Channel ch)
{
    auto index = static_cast<size_t>(cp) + (ch == Channel::Left ? 0 : 8);
    jassert(index >= 0 && index <= 16);
    return index;
}

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

float qFromBandWidth(float BW)
{
    // q from bandwidth in octaves, assume sufficiently high sample rate.
    // https://www.w3.org/TR/audio-eq-cookbook/
    
    double invQ = 2 * std::sinh(std::log(2.0) / 2.0 * BW);
    if(invQ > 0)
        return 1.0 / invQ;
    else
        return 10.f;
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
    auto channelLabel = [&]()
    {
        Channel ch;
        switch (widgetVar.component.index())
        {
            case WidgetVariant::Node:
            {
                auto node = std::get<AnalyzerNode*>(widgetVar.component);
                ch = node->getChannel();
                break;
            }
                
            case WidgetVariant::Band:
            {
                auto band = std::get<AnalyzerBand*>(widgetVar.component);
                ch = band->getChannel();
                break;
            }
            case WidgetVariant::QControl:
            {
                auto qcontrol = std::get<AnalyzerQControl *>(widgetVar.component);
                ch = qcontrol->getChannel();
            }
            default:
                ch = Channel::Left;
        }
        return ch == Channel::Left ? "L" : "R";
    }();
    
 
    int x = 0;
    int y = 0;
    if(widgetVar.component.index() != WidgetVariant::Controller)
    {
        x = event.originalComponent->getX();
        y = event.originalComponent->getY();
    }
    
     
    
    DBG(type + " : " + componentID + " ch:" + channelLabel + " freq:" +
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
        
        addWidget(i, bands, "BAND", pos, Channel::Left);
        addWidget(i + 8, bands, "BAND", pos, Channel::Right);
       
        addWidget(i, nodes, "NODE", pos, Channel::Left, true);
        addWidget(i + 8, nodes, "NODE", pos, Channel::Right, true);
        
        addAttachments(i, pos, Channel::Left);
        addAttachments(i + 8, pos, Channel::Right);
    }
    
 
    qControlLeft.setComponentID(juce::String("Left QControl"));
    qControlRight.setComponentID(juce::String("Right QControl"));
    qControlLeft.addMouseListener(this, false);
    qControlRight.addMouseListener(this, false);
    addChildComponent(qControlLeft);
    addChildComponent(qControlRight);
    
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
    if(qControlsVisible())
        refreshQControls();
}


void NodeController::refreshQControls()
{
    if(currentBand)
    {
        auto bandBounds = currentBand->getBounds();
        auto leftBounds = bandBounds.withWidth(4);
        auto rightBounds = bandBounds.withWidth(4);
        leftBounds.setCentre(bandBounds.getX(), bandBounds.getCentreY());
        rightBounds.setCentre(bandBounds.getRight(), bandBounds.getCentreY());
        qControlLeft.setBounds(leftBounds);
        qControlRight.setBounds(rightBounds);
        
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
    
    
    bounds.setWidth(bBox.getWidth() * BW / widthOctaves);
    
    bounds.setCentre(centerX, bBox.getCentreY());
    
    band.setBounds(bounds.toNearestInt());
    band.setVisible(true);
}


// Mouse Handling
void NodeController::mouseMove(const juce::MouseEvent &event)
{
    
}


 
void NodeController::mouseEnter(const juce::MouseEvent &event)
{
  
    auto widgetVar = getEventsComponent(event);
     
    switch (widgetVar.component.index())
    {
        case WidgetVariant::Node:
        {
            auto node = std::get<AnalyzerNode*>(widgetVar.component);
            node->displayAsSelected(true);
            nodeListeners.call([&node](Listener& nl){nl.bandMousedOver(node->getChainPosition(),
                                                                           node->getChannel());});
            
            auto index = getWidgetIndex(node->getChainPosition(), node->getChannel());
            bands[index]->displayAsSelected(true);
            if(!qControlsVisible())
                bands[index]->toFront(false);
            
            break;
        }
            
        case WidgetVariant::Band:
        {
            auto band = std::get<AnalyzerBand*>(widgetVar.component);
            band->displayAsSelected(true);
            if(!qControlsVisible())
                band->toFront(false);
    
            nodeListeners.call([&band](Listener& nl){nl.bandMousedOver(band->getChainPosition(),
                                                                           band->getChannel());});
            break;
        }
            
        case WidgetVariant::QControl:
        {
            auto qControl = std::get<AnalyzerQControl*>(widgetVar.component);
            qControl->displayAsSelected(true);
            qControl->toFront(false);
            break;
        }
            
        default:
            ;
            
    }
    debugMouse("Enter", event);
}


void NodeController::mouseExit(const juce::MouseEvent &event)
{
    auto widgetVar = getEventsComponent(event);
    // use a visitor here?
    switch (widgetVar.component.index())
    {
        case WidgetVariant::Node:
        {
            auto node = std::get<AnalyzerNode*>(widgetVar.component);
            //if(!qControlsVisible() || currentNode != node)
            node->displayAsSelected(false);
            
            break;
        }
            
        case WidgetVariant::Band:
        {
            auto band = std::get<AnalyzerBand*>(widgetVar.component);
            if(!qControlsVisible() || currentBand != band)
            {
                band->displayAsSelected(false);
                nodeListeners.call([](Listener& nl){nl.clearSelection();});
            }
            break;
        }
            
        case WidgetVariant::QControl:
        {
            auto qControl = std::get<AnalyzerQControl*>(widgetVar.component);
            qControl->displayAsSelected(false);
            break;
        }
            
        case WidgetVariant::Controller:
        {
         
            if(currentNode && !fftBoundingBox.toFloat().contains(event.position))
                deactivateQControls();
             
            break;
        }
          
            
        default:
            ;
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
            getAttachmentForWidget(freqAttachements, node).beginGesture();
            getAttachmentForWidget(gainOrSlopeAttachements, node).beginGesture();
            
            //  activate q controls on this band.
            if(currentNode && currentNode != node)
                deactivateQControls();
            
            currentNode = node;
            currentBand = bands[getWidgetIndex(node->getChainPosition(), node->getChannel())].get();
            activateQControls(node->getChainPosition(), node->getChannel());
            nodeListeners.call([&node](Listener& nl){nl.bandSelected(node->getChainPosition(),
                                                                           node->getChannel());});
            break;
        }
            
        case WidgetVariant::Band:
        {
            auto band = std::get<AnalyzerBand*>(widgetVar.component);
            dragger.startDraggingComponent(band, event);
            getAttachmentForWidget(freqAttachements, band).beginGesture();
            if(currentNode && (currentNode->getChannel() != band->getChannel() || currentNode->getChainPosition() != band->getChainPosition()))
            {
                deactivateQControls();
            }
            break;
        }
        
        case WidgetVariant::QControl:
        {
            auto qControl = std::get<AnalyzerQControl*>(widgetVar.component);
            dragger.startDraggingComponent(qControl, event);
            getAttachmentForWidget(qAttachements, qControl).beginGesture();
            break;
        }
            
        default:
            if(currentNode)
                deactivateQControls();
            
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
            auto& gainOrSlopeAttach = getAttachmentForWidget(gainOrSlopeAttachements, node);
            
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
            
            getAttachmentForWidget(freqAttachements, node).setValueAsPartOfGesture(node->getFrequency());
            
            break;
        }
        
        case WidgetVariant::Band:
        {
            auto band = std::get<AnalyzerBand*>(widgetVar.component);
            dragger.dragComponent(band, event, &hConstrainer);
            
            float freq = frequencyFromX(band->getBounds().getCentreX());
            
            getAttachmentForWidget(freqAttachements, band).setValueAsPartOfGesture(freq);
            break;
        }
        
        case WidgetVariant::QControl:
        {
            auto qControl = std::get<AnalyzerQControl*>(widgetVar.component);
            dragger.dragComponent(qControl, event, &hConstrainer);
            if(currentBand)
            {
                auto bandBounds = currentBand->getBounds();
                auto newWidth = std::abs(bandBounds.getCentreX() - qControl->getBounds().getCentreX()) * 2.0f;
                auto BW = newWidth / fftBoundingBox.getWidth() * widthOctaves;
                getAttachmentForWidget(qAttachements, qControl).setValueAsPartOfGesture(qFromBandWidth(BW));
                refreshQControls(); // forces current qControl to snap, listener misses it.
            }
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
            getAttachmentForWidget(freqAttachements, node).endGesture();
            getAttachmentForWidget(gainOrSlopeAttachements, node).endGesture();
            break;
        }
            
        case WidgetVariant::Band:
        {
            auto band = std::get<AnalyzerBand *>(widgetVar.component);
            getAttachmentForWidget(freqAttachements, band).endGesture();
            break;
        }
        
        case WidgetVariant::QControl:
        {
            auto qControl = std::get<AnalyzerQControl *>(widgetVar.component);
            getAttachmentForWidget(qAttachements, qControl).endGesture();
            break;
        }
        default:
            ;
            
    }
    debugMouse("Mouse Up", event);
}

void NodeController::mouseDoubleClick(const juce::MouseEvent &event)
{
    auto widgetVar = getEventsComponent(event);
 
    switch (widgetVar.component.index())
    {
        case WidgetVariant::Node:
        {
            auto node = std::get<AnalyzerNode*>(widgetVar.component);
            // TODO reset freq and q
            
            break;
        }
            
        case WidgetVariant::Band:
        {
            auto band = std::get<AnalyzerBand*>(widgetVar.component);
            // TODO reset frequency only
            break;
        }
            
        case WidgetVariant::QControl:
        {
            auto qControl = std::get<AnalyzerQControl*>(widgetVar.component);
            // TODO reset Q only
            break;
        }
            
        case WidgetVariant::Controller:
        {
            resetAllParameters();
            break;
        }
          
            
        default:
            ;
    }
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


ParameterAttachment& NodeController::getAttachmentForWidget(std::array<std::unique_ptr<ParameterAttachment>, 16>& attachments, AnalyzerWidgetBase* node)
{
    Channel ch = node->getChannel();
    ChainPosition cp = node->getChainPosition();
    size_t filterNum = static_cast<size_t>(cp);
    
    if(ch == Channel::Left)
        return *attachments[filterNum];
 
    return *attachments[filterNum + 8];
}


void NodeController::activateQControls(ChainPosition pos, Channel ch)
{
    if(currentNode)
    {
        qControlLeft.setChannel(ch);
        qControlLeft.setChainPosition(pos);
        qControlLeft.setVisible(true);
        qControlLeft.toFront(false);
        qControlRight.setChannel(ch);
        qControlRight.setChainPosition(pos);
        qControlRight.setVisible(true);
        qControlRight.toFront(false);
        refreshQControls();
    }
}


void NodeController::deactivateQControls()
{
    qControlLeft.setVisible(false);
    qControlRight.setVisible(false);
    currentNode->displayAsSelected(false);
    if(currentBand)
    {
        currentBand->displayAsSelected(false);
        nodeListeners.call([](Listener& nl){nl.clearSelection();});
    }
}

void NodeController::resetAllParameters()
{
    
    auto setDefaults = [&](size_t i, ChainPosition cp)
    {
        using namespace ChainHelpers;
        qAttachements[i]->setValueAsCompleteGesture(defaultQ.at(cp));
        freqAttachements[i]->setValueAsCompleteGesture(defaultFrequencies.at(cp));
        if(cp == ChainPosition::LowCut || cp == ChainPosition::HighCut)
             gainOrSlopeAttachements[i]->setValueAsCompleteGesture(defaultSlopeIndex);
        else
            gainOrSlopeAttachements[i]->setValueAsCompleteGesture(defaultGain);
    };
    
    for(size_t j=0;j < 8; ++j)
    {
        ChainPosition cp = static_cast<ChainPosition>(j);
        setDefaults(j, cp);
        setDefaults(j+8, cp);
        
    }
      
}


void NodeController::addNodeListener (Listener* lsnr)
{
    nodeListeners.add(lsnr);
}
 
 
void NodeController::removeNodeListener (Listener* lsnr)
{
    jassert(nodeListeners.contains(lsnr));
    nodeListeners.remove(lsnr);
}


