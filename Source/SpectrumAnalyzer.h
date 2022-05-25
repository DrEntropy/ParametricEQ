/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 17 May 2022 10:34:23pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AnalyzerProperties.h"
#include "AnalyzerBase.h"
#include "PathProducer.h"
#include "SingleChannelSampleFifo.h"
#include "DbScale.h"
#include "ParamListener.h"
#include "EQConstants.h"

#define RESPONSE_CURVE_MIN_DB -30.f
#define RESPONSE_CURVE_MAX_DB 30.f

template<typename BlockType>
struct SpectrumAnalyzer : AnalyzerBase, juce::Timer
{
    SpectrumAnalyzer(double sr,
                     SingleChannelSampleFifo<BlockType>& leftScsf,
                     SingleChannelSampleFifo<BlockType>& rightScsf,
                     juce::AudioProcessorValueTreeState& apv);
    void timerCallback() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void customizeScales(int leftScaleMin, int leftScaleMax, int rightScaleMin, int rightScaleMax, int division);
    void changeSampleRate(double sr);
    
private:
    double sampleRate;
    juce::Path leftAnalyzerPath, rightAnalyzerPath;
    
    PathProducer<BlockType> leftPathProducer, rightPathProducer;
    
    bool active { false };
    
    void paintBackground(juce::Graphics&);
    
    void setActive(bool a);
    void updateDecayRate(float dr);
    void updateOrder(float);
    void animate();
    
    DbScale analyzerScale, eqScale;
    
    std::unique_ptr<ParamListener> analyzerEnabledParamListener,
                                   analyzerDecayRateParamListener,
                                   analyzerOrderParamListener;
    
    float leftScaleMin {RESPONSE_CURVE_MIN_DB - 30.f}, leftScaleMax {RESPONSE_CURVE_MAX_DB - 30.f}, rightScaleMin{RESPONSE_CURVE_MIN_DB}, rightScaleMax{RESPONSE_CURVE_MAX_DB};
    int scaleDivision { 6 };
};
