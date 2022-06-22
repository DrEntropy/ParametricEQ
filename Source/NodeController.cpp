/*
  ==============================================================================

    NodeController.cpp
    Created: 30 May 2022 10:10:21am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "NodeController.h"
#include "ChainHelpers.h"
#include "GlobalParameters.h"
#include <variant>

// helper type for the variant visitor
template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

// monostate is the default variant and indicates an invalid state in this model.
using WidgetVariant = std::variant<std::monostate, NodeController*, AnalyzerNode*, AnalyzerQControl*, AnalyzerBand*>;
 
 
size_t getWidgetIndex(ChainPosition cp, Channel ch)
{
    auto index = static_cast<size_t>(cp) + (ch == Channel::Left ? 0 : 8);
    jassert(index >= 0 && index <= 16);
    return index;
}

WidgetVariant getEventsComponent(const juce::MouseEvent &event)
{
    auto componentPtr =  event.originalComponent;
    
    if(auto nodeController = dynamic_cast<NodeController*>(componentPtr))
    {
        return nodeController;
    }
    else if(auto analyzerNode = dynamic_cast<AnalyzerNode *>(componentPtr))
    {
        return  analyzerNode;
    }
    else if(auto analyzerQ = dynamic_cast<AnalyzerQControl *>(componentPtr))
    {
        return analyzerQ;
    }
    else if(auto band = dynamic_cast<AnalyzerBand *>(componentPtr))
    {
        return band;
    }
    else
    {
       jassertfalse;  // Should not happen
       return std::monostate();
    }
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
    if(!MOUSE_DEBUG)
        return;
    
    auto componentID =  event.originalComponent -> getComponentID();
    auto widgetVar = getEventsComponent(event);
    
    auto channelToString = [](Channel ch) { return ch == Channel::Left ? "L" : "R";};
    
    juce::String channelLabel;
    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
            channelLabel = channelToString(node->getChannel());
        },
        [&](AnalyzerBand* band)
        {
            channelLabel = channelToString(band->getChannel());
        },
        [&](AnalyzerQControl* qControl)
        {
            channelLabel = channelToString(qControl->getChannel());
        },
        [&](NodeController* /* controller */)
        {
            channelLabel = "NA";
        },
        [](std::monostate) { jassertfalse; }
    }, widgetVar);
    
    auto x = event.originalComponent->getX();
    auto y = event.originalComponent->getY();
  
    DBG(type + " : " + componentID + " ch:" + channelLabel + " freq:" +
        std::to_string(frequencyFromX(x + event.x))  + " gain:" + std::to_string(gainFromY(y + event.y)) );
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
    bool drawRightMid = static_cast<ChannelMode>(apvts.getRawParameterValue(GlobalParameters::processingModeName)->load()) != ChannelMode::Stereo;
    
    for(uint i=0; i< 8; ++i)
    {
        updateNode(*nodes[i], bBox);
        updateBand(*bands[i], bBox);

        if(drawRightMid)
        {
            updateNode(*nodes[i + 8], bBox);
            updateBand(*bands[i + 8], bBox);
        }
        else
        {
            nodes[i+8]->setVisible(false);
            bands[i+8]->setVisible(false);
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

 


 
void NodeController::mouseEnter(const juce::MouseEvent &event)
{
  
    auto widgetVar = getEventsComponent(event);
    
    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
            node->displayAsSelected(true);
            nodeListeners.call([&node](Listener& nl){nl.bandMousedOver(node->getChainPosition(),
                                                                       node->getChannel());});
            
            auto index = getWidgetIndex(node->getChainPosition(), node->getChannel());
            bands[index]->displayAsSelected(true);
            if(!qControlsVisible())
                bands[index]->toFront(false);
            
        },
        [&](AnalyzerBand* band)
        {
            band->displayAsSelected(true);
            if(!qControlsVisible())
                band->toFront(false);
    
            nodeListeners.call([&band](Listener& nl){nl.bandMousedOver(band->getChainPosition(),
                                                                       band->getChannel());});
        },
        [&](AnalyzerQControl* qControl)
        {
            qControl->displayAsSelected(true);
            qControl->toFront(false);
        },
        [](NodeController* /* controller */) {},
        [](std::monostate) { jassertfalse; }
            }, widgetVar);
     
    debugMouse("Enter", event);
}


void NodeController::mouseExit(const juce::MouseEvent &event)
{
    auto widgetVar = getEventsComponent(event);
    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
            node->displayAsSelected(false);
        },
        [&](AnalyzerBand* band)
        {
            if(!qControlsVisible() || currentBand != band)
            {
                band->displayAsSelected(false);
                nodeListeners.call([](Listener& nl){nl.clearSelection();});
            }
        },
        [&](AnalyzerQControl* qControl)
        {
            qControl->displayAsSelected(false);
        },
        [&](NodeController* /* controller */)
        {
            if(currentNode && !fftBoundingBox.toFloat().contains(event.position))
                deactivateQControls();
        },
        [](std::monostate) { jassertfalse; }
            }, widgetVar);
     
    debugMouse("Exit", event);
}

void NodeController::mouseDown(const juce::MouseEvent &event)
{
    
    auto widgetVar = getEventsComponent(event);
    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
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
        },
        [&](AnalyzerBand* band)
        {
            auto node = nodes[getWidgetIndex(band->getChainPosition(), band->getChannel())].get();
            dragger.startDraggingComponent(node, event);
            getAttachmentForWidget(freqAttachements, node).beginGesture();
            getAttachmentForWidget(gainOrSlopeAttachements, node).beginGesture();
            
            if(qControlsVisible() && currentNode && (currentNode->getChannel() != band->getChannel()
                                                || currentNode->getChainPosition() != band->getChainPosition()))
            {
                deactivateQControls();
                nodeListeners.call([&band](Listener& nl){nl.bandSelected(band->getChainPosition(),
                                                                               band->getChannel());});
            }
        },
        [&](AnalyzerQControl* qControl)
        {
            dragger.startDraggingComponent(qControl, event);
            getAttachmentForWidget(qAttachements, qControl).beginGesture();
        },
        [&](NodeController* /* controller */)
        {
            if(currentNode)
                deactivateQControls();
        },
        [](std::monostate) { jassertfalse; }
            }, widgetVar);
     
    debugMouse("Down", event);
}

void NodeController::mouseDrag(const juce::MouseEvent &event)
{
    
    auto widgetVar = getEventsComponent(event);
    
    auto dragNode = [&](AnalyzerNode* node)
    {
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
        
        updateNode(*node, fftBoundingBox.toFloat());
    };

    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
            dragNode(node);
        },
        [&](AnalyzerBand* band)
        {
            auto node = nodes[getWidgetIndex(band->getChainPosition(), band->getChannel())].get();
            dragNode(node);
        },
        [&](AnalyzerQControl* qControl)
        {
            dragger.dragComponent(qControl, event, &hConstrainer);
            if(currentBand)
            {
                auto bandBounds = currentBand->getBounds();
                auto newWidth = std::abs(bandBounds.getCentreX() - qControl->getBounds().getCentreX()) * 2.0f;
                auto BW = newWidth / fftBoundingBox.getWidth() * widthOctaves;
                getAttachmentForWidget(qAttachements, qControl).setValueAsPartOfGesture(qFromBandWidth(BW));
                refreshQControls(); // forces current qControl to snap, listener misses it.
            }
        },
        [&](NodeController* /* controller */)
        {
           // Nothing
        },
        [](std::monostate) { jassertfalse; }
    }, widgetVar);
    
    
   
   // debugMouse("Drag", event);
}

void NodeController::mouseUp(const juce::MouseEvent &event)
{
    auto widgetVar = getEventsComponent(event);
    
    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
            getAttachmentForWidget(freqAttachements, node).endGesture();
            getAttachmentForWidget(gainOrSlopeAttachements, node).endGesture();
        },
        [&](AnalyzerBand* band)
        {
            auto node = nodes[getWidgetIndex(band->getChainPosition(), band->getChannel())].get();
            getAttachmentForWidget(freqAttachements, node).endGesture();
            getAttachmentForWidget(gainOrSlopeAttachements, node).endGesture();
        },
        [&](AnalyzerQControl* qControl)
        {
            getAttachmentForWidget(qAttachements, qControl).endGesture();
        },
        [&](NodeController* /* controller */)
        {
           // Nothing
        },
        [](std::monostate) { jassertfalse; }
    }, widgetVar);
    
    debugMouse("Mouse Up", event);
}

void NodeController::mouseDoubleClick(const juce::MouseEvent &event)
{
    auto widgetVar = getEventsComponent(event);
    
    std::visit(Overloaded
    {
        [&](AnalyzerNode* node)
        {
            auto cp = node->getChainPosition();
            auto ch = node->getChannel();
            resetFreq(cp, ch);
            resetGainOrSlope(cp, ch);
        },
        [&](AnalyzerBand* band)
        {
            auto cp = band->getChainPosition();
            auto ch = band->getChannel();
            resetFreq(cp, ch);
        },
        [&](AnalyzerQControl* qControl)
        {
            auto cp = qControl->getChainPosition();
            auto ch = qControl->getChannel();
            resetQ(cp, ch);
        },
        [&](NodeController* /* controller */)
        {
            resetAllParameters();
        },
        [](std::monostate) { jassertfalse; }
    }, widgetVar);
 
               
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
    //currentNode->displayAsSelected(false);
    if(currentBand)
    {
        currentBand->displayAsSelected(false);
        nodeListeners.call([](Listener& nl){nl.clearSelection();});
    }
}
void NodeController::resetQ(ChainPosition cp, Channel ch)
{
    using namespace ChainHelpers;
    auto index =  getWidgetIndex(cp, ch);
    qAttachements[index]->setValueAsCompleteGesture(defaultQ.at(cp));
}

void NodeController::resetFreq(ChainPosition cp, Channel ch)
{
    using namespace ChainHelpers;
    auto index =  getWidgetIndex(cp, ch);
    freqAttachements[index]->setValueAsCompleteGesture(defaultFrequencies.at(cp));
}

void NodeController::resetGainOrSlope(ChainPosition cp, Channel ch)
{
    using namespace ChainHelpers;
    auto index =  getWidgetIndex(cp, ch);
    if(cp == ChainPosition::LowCut || cp == ChainPosition::HighCut)
        gainOrSlopeAttachements[index]->setValueAsCompleteGesture(defaultSlopeIndex);
    else
        gainOrSlopeAttachements[index]->setValueAsCompleteGesture(defaultGain);
}

void NodeController::resetAllParameters()
{
    for(size_t j=0;j < 8; ++j)
    {
        ChainPosition cp = static_cast<ChainPosition>(j);
        resetGainOrSlope(cp, Channel::Left);
        resetFreq(cp, Channel::Left);
        resetQ(cp, Channel::Left);
        
        resetGainOrSlope(cp, Channel::Right);
        resetFreq(cp, Channel::Right);
        resetQ(cp, Channel::Right);
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


