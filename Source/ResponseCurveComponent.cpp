/*
  ==============================================================================

    ResponseCurveComponent.cpp
    Created: 26 May 2022 11:52:34am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ResponseCurveComponent.h"
#include "Para"

ResponseCurveComponent::ResponseCurveComponent(double sr, juce::AudioProcessorValueTreeState& apvtsIn) : apvts{apvtsIn}, sampleRate{sr}
{
  
//TODO: VERIFY if this is needed.
//  auto safePtr = juce::Component::SafePointer<ResponseCurveComponent>(this);
//    allParamsListener.reset( new AllParamsListener(apvts,
//                                                   [safePtr]()
//                                                   {
//                                                       if(auto* comp = safePtr.getComponent() )
//                                                           comp->refreshParams();
//                                                   }));
    
    allParamsListener.reset( new AllParamsListener(apvts, std::bind( &ResponseCurveComponent::refreshParams, this)));
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    //use a Graphics::ScopedSaveState to reduce the clipping region to the fftBoundingBox (?)
 
    g.reduceClipRegion(fftBoundingBox);

    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    
    g.setColour(juce::Colours::yellow);
    g.strokePath(leftResponseCurve, pst);
    
    if(static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load()) != ChannelMode::Stereo)
    {
        g.setColour(juce::Colours::hotpink);
        g.strokePath(rightResponseCurve, pst);
    }
    
}

void ResponseCurveComponent::resized()
{
    AnalyzerBase::resized();
    buildNewResponseCurves();
}

void ResponseCurveComponent::refreshParams()
{
    buildNewResponseCurves();
    repaint();
}

void ResponseCurveComponent::buildNewResponseCurves()
{
    updateChainParameters();
 
    leftResponseCurve = createResponseCurve(buildNewResponseCurve(leftChain));
    
    if(static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load()) != ChannelMode::Stereo)
    {
        rightResponseCurve = createResponseCurve(buildNewResponseCurve(rightChain));
    }
}

void updateChainParameters()
{
}

std::vector<float> buildNewResponseCurve(MonoFilterChain& chain)
{
    
}

juce::Path createResponseCurve(const std::vector<float>& data)
{
    
}
