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
    // PLACEHOLDER FOR TESTING,
    leftPathProducer.setDecayRate(120.f);
    leftPathProducer.changeOrder(AnalyzerProperties::FFTOrder::FFT4096);
    animate();
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::timerCallback()
{
    if(leftPathProducer.getNumAvailableForReading()>0)
        repaint();
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::resized()
{
    leftPathProducer.setFFTRectBounds(getBoundsForFFT().toFloat());
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::red);
    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    juce::Path fftPath;
    
    auto centerBounds = getBoundsForFFT();
    
    //g.reduceClipRegion(centerBounds.toNearestInt());
    
    if(leftPathProducer.getNumAvailableForReading() > 0)
    {
        while(leftPathProducer.getNumAvailableForReading() > 0)
            leftPathProducer.pull(fftPath);
        
        g.strokePath(fftPath, pst);
    }
    
    g.setColour(juce::Colours::lightblue);
    g.drawRect(centerBounds);
    
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::customizeScales(int leftScaleMin, int leftScaleMax, int rightScaleMin, int rightScaleMax, int division)
{
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::changeSampleRate(double sr)
{
    leftPathProducer.updateSampleRate(sr);
    rightPathProducer.updateSampleRate(sr);
}
 
template <typename BlockType>
void SpectrumAnalyzer<BlockType>::paintBackground(juce::Graphics&)
{
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::setActive(bool a)
{
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::updateDecayRate(float dr)
{
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::updateOrder(float value)
{
    
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::animate()
{
    startTimerHz(FRAME_RATE);
}

 
template struct SpectrumAnalyzer<juce::AudioBuffer<float> >;
