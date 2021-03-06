/*
  ==============================================================================

    ResponseCurveComponent.cpp
    Created: 26 May 2022 11:52:34am
    Author:  Ronald Legere

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ResponseCurveComponent.h"
#include "EQConstants.h"
#include "GlobalParameters.h"

using namespace ChainHelpers;

ResponseCurveComponent::ResponseCurveComponent(double sr, juce::AudioProcessorValueTreeState& apvtsIn) : apvts{apvtsIn}, sampleRate{sr}
{
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
    
    if(static_cast<ChannelMode>(apvts.getRawParameterValue(GlobalParameters::processingModeName)->load()) != ChannelMode::Stereo)
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
 
    createResponseCurve(leftResponseCurve, buildNewResponseCurve(leftChain));
    
    if(static_cast<ChannelMode>(apvts.getRawParameterValue(GlobalParameters::processingModeName)->load()) != ChannelMode::Stereo)
    {
        createResponseCurve(rightResponseCurve, buildNewResponseCurve(rightChain));
    }
}

void ResponseCurveComponent::updateChainParameters()
{
    ChainHelpers::initializeFilters(leftChain, Channel::Left, apvts, 0.0, false, sampleRate);
    ChainHelpers::initializeFilters(rightChain, Channel::Right, apvts, 0.0, false, sampleRate);
}

 

std::vector<float> ResponseCurveComponent::buildNewResponseCurve(MonoFilterChain& chain)
{
    auto w =  static_cast<size_t>(fftBoundingBox.getWidth());
    
    std::vector<float> path(w, NEGATIVE_INFINITY);
    
    for(size_t x = 0; x < w; ++x )
    {
        double mag {1.0};
        double freq = juce::mapToLog10(static_cast<double>(x) / w, 20.0, 20000.0);
        
        mag *= chain.get<0>().isBypassed() ? 1.0 : chain.get<0>().getCutFilterMagnitudeForFrequency(freq);
        mag *= chain.get<1>().isBypassed() ? 1.0 : chain.get<1>().getFilterMagnitudeForFrequency(freq);
        mag *= chain.get<2>().isBypassed() ? 1.0 : chain.get<2>().getFilterMagnitudeForFrequency(freq);
        mag *= chain.get<3>().isBypassed() ? 1.0 : chain.get<3>().getFilterMagnitudeForFrequency(freq);
        mag *= chain.get<4>().isBypassed() ? 1.0 : chain.get<4>().getFilterMagnitudeForFrequency(freq);
        mag *= chain.get<5>().isBypassed() ? 1.0 : chain.get<5>().getFilterMagnitudeForFrequency(freq);
        mag *= chain.get<6>().isBypassed() ? 1.0 : chain.get<6>().getFilterMagnitudeForFrequency(freq);
        mag *= chain.get<7>().isBypassed() ? 1.0 : chain.get<7>().getCutFilterMagnitudeForFrequency(freq);
        
        path[x] = juce::Decibels::gainToDecibels(static_cast<float>(mag), NEGATIVE_INFINITY);
    }
    return path;
}

void ResponseCurveComponent::createResponseCurve(juce::Path& path, const std::vector<float>& data)
{
    if(data.empty())
        return;
    
    path.clear();
    
    auto startX = fftBoundingBox.getX();
    
    auto mapGain = [this](float gain)
    {
        auto bBox = fftBoundingBox.toFloat();
        return juce::jmap(gain, static_cast<float>(RESPONSE_CURVE_MIN_DB), static_cast<float>(RESPONSE_CURVE_MAX_DB), bBox.getBottom(), bBox.getY());
    };

    path.startNewSubPath(startX,  mapGain(data[0]));
    
    for(size_t i = 1; i < data.size(); ++i)
    {
        path.lineTo(i + startX, mapGain(data[i]));
    }

    return ;
}
