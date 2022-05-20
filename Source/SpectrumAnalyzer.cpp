/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 17 May 2022 10:34:23pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"

template <typename BlockType>
SpectrumAnalyzer<BlockType>::SpectrumAnalyzer(double sr,
                 SingleChannelSampleFifo<BlockType>& leftScsf,
                 SingleChannelSampleFifo<BlockType>& rightScsf,
                                   juce::AudioProcessorValueTreeState& apv) : sampleRate{sr}, leftPathProducer {sr, leftScsf}, rightPathProducer {sr, rightScsf}
{
    using namespace AnalyzerProperties;
    
    
    auto getParam = [&apv](ParamNames param)
    {
        return apv.getParameter(getAnalyzerParamName(param));
    };
    
    
    auto safePtr = juce::Component::SafePointer<SpectrumAnalyzer<BlockType>>(this);
    
    analyzerEnabledParamListener.reset(new ParamListener(getParam(ParamNames::EnableAnalyzer),
                                                         [safePtr](float v)
                                                         {
                                                             if(auto* comp = safePtr.getComponent() )
                                                                comp->setActive(v > 0.5);
                                                         }));
    
  
    analyzerDecayRateParamListener.reset(new ParamListener(getParam(ParamNames::AnalyzerDecayRate),
                                                           [safePtr](float v)
                                                           {
                                                               if(auto* comp = safePtr.getComponent() )
                                                                  comp->updateDecayRate(v);
                                                           }));
   
    analyzerOrderParamListener.reset(new ParamListener(getParam(ParamNames::AnalyzerPoints),
                                                       [safePtr](float v)
                                                       {
                                                           if(auto* comp = safePtr.getComponent() )
                                                              comp->updateOrder(v);
                                                       }));
    
    updateDecayRate(apv.getRawParameterValue(getAnalyzerParamName(ParamNames::AnalyzerDecayRate))->load());
    
    updateOrder(apv.getRawParameterValue(getAnalyzerParamName(ParamNames::AnalyzerPoints))->load());
    setActive(apv.getRawParameterValue(getAnalyzerParamName(ParamNames::EnableAnalyzer))->load() > 0.5);
    
    addAndMakeVisible(eqScale);
    addAndMakeVisible(analyzerScale);
    
    animate();
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::timerCallback()
{
    if(!active)
    {
        leftAnalyzerPath.clear();
        rightAnalyzerPath.clear();
        stopTimer();
    }
    else
    {
    while(leftPathProducer.getNumAvailableForReading() > 0)
        leftPathProducer.pull(leftAnalyzerPath);
    
    while(rightPathProducer.getNumAvailableForReading() > 0)
        rightPathProducer.pull(rightAnalyzerPath);
    }
    repaint();
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::resized()
{
    AnalyzerBase::resized();
    leftPathProducer.setFFTRectBounds(fftBoundingBox.toFloat());
    rightPathProducer.setFFTRectBounds(fftBoundingBox.toFloat());
    
    auto bounds =  getLocalBounds();
    auto eqScaleBounds = bounds.removeFromRight(getTextWidthScaled());
    auto analyzerScaleBounds = bounds.removeFromLeft(getTextWidthScaled());
    
    eqScale.setBounds(eqScaleBounds);
   
    analyzerScale.setBounds(analyzerScaleBounds);
    
    customizeScales(leftScaleMin, leftScaleMax, rightScaleMin, rightScaleMax, scaleDivision);
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::paint(juce::Graphics& g)
{
    paintBackground(g);
    g.reduceClipRegion(fftBoundingBox);
    g.setColour(juce::Colours::red);
    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    
    g.strokePath(leftAnalyzerPath, pst);
    
    g.setColour(juce::Colours::salmon);
    g.strokePath(rightAnalyzerPath, pst);
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::customizeScales(int leftMin, int leftMax, int rightMin, int rightMax, int division)
{
    leftScaleMin = leftMin;
    rightScaleMin = rightMin;
    leftScaleMax = leftMax;
    rightScaleMax = rightMax;
    scaleDivision = division;
    
    leftPathProducer.changePathRange(leftMin, leftMax);
    rightPathProducer.changePathRange(leftMin, leftMax);
    eqScale.buildBackgroundImage(scaleDivision, fftBoundingBox, rightScaleMin, rightScaleMax);
    analyzerScale.buildBackgroundImage(scaleDivision, fftBoundingBox, leftScaleMin, leftScaleMax);
    
    if(! getLocalBounds().isEmpty())
       repaint();  // verify that this doesnt cause a double call to repaint when this is called from resized.
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::changeSampleRate(double sr)
{
    sampleRate = sr;
    leftPathProducer.updateSampleRate(sr);
    rightPathProducer.updateSampleRate(sr);
}
 
template <typename BlockType>
void SpectrumAnalyzer<BlockType>::paintBackground(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightblue);
    g.drawRect(getLocalBounds().toFloat());
    // TODO: draw scale lines
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::setActive(bool a)
{
    active = a;
    
    if(active && !isTimerRunning())
        animate();
        
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::updateDecayRate(float dr)
{
    leftPathProducer.setDecayRate(dr);
    rightPathProducer.setDecayRate(dr);
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::updateOrder(float v)
{
    using AnalyzerProperties::FFTOrder;
    int lowest = static_cast<int>(FFTOrder::FFT2048);
    FFTOrder o = static_cast<FFTOrder>(v + lowest);
    leftPathProducer.changeOrder(o);
    rightPathProducer.changeOrder(o);
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::animate()
{
    startTimerHz(FRAME_RATE);
}

 
template struct SpectrumAnalyzer<juce::AudioBuffer<float> >;
