/*
  ==============================================================================

    ResponseCurveComponent.h
    Created: 26 May 2022 11:52:34am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AnalyzerBase.h"
#include "AllParamsListener.h"
#include "PluginProcessor.h"
#include "ChainHelpers.h"

 
struct ResponseCurveComponent : AnalyzerBase
{
    ResponseCurveComponent(double sr,
                           juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    double sampleRate;
    std::unique_ptr<AllParamsListener> allParamsListener;
    
    ChainHelpers::MonoFilterChain leftChain, rightChain;
    juce::Path leftResponseCurve, rightResponseCurve;
    
    void refreshParams();
    void buildNewResponseCurves();
    void updateChainParameters();
    std::vector<float> buildNewResponseCurve(ChainHelpers::MonoFilterChain& chain);
    void createResponseCurve(juce::Path& path, const std::vector<float>& data);
};
