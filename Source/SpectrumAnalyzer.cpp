/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 17 May 2022 10:34:23pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"

template <typename BlockType>
SpectrumAnalyzer<BlockType>::SpectrumAnalyzer(double sr, SingleChannelSampleFifo<BlockType>& leftScsf, SingleChannelSampleFifo<BlockType>& rightScsf,
                                              juce::AudioProcessorValueTreeState& apv) : sampleRate{sr}, leftPathProducer {sr, leftScsf},
                                              rightPathProducer {sr, rightScsf}
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
    auto eqScaleBounds = bounds.removeFromRight(getScaleWidth());
    auto analyzerScaleBounds = bounds.removeFromLeft(getScaleWidth());
    
    eqScale.setBounds(eqScaleBounds);
    analyzerScale.setBounds(analyzerScaleBounds);
    
    customizeScales(leftScaleMin, leftScaleMax, rightScaleMin, rightScaleMax, scaleDivision);
}

template <typename BlockType>
void SpectrumAnalyzer<BlockType>::paint(juce::Graphics& g)
{
    paintBackground(g);
    g.reduceClipRegion(fftBoundingBox);

    juce::PathStrokeType pst(2, juce::PathStrokeType::curved);
    
    g.setColour(juce::Colours::red);
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
    
    
    g.setOpacity(0.5f);
    
    // Draw scale lines
    auto currentDb = leftScaleMin;
    
    while(currentDb <= leftScaleMax)
    {
        auto y = juce::jmap(currentDb, leftScaleMin, leftScaleMax, fftBoundingBox.toFloat().getBottom(), fftBoundingBox.toFloat().getY());
        g.setColour(currentDb == 0 ? juce::Colours::blue : juce::Colours::lightgrey);
        g.drawLine(fftBoundingBox.getX(), y, fftBoundingBox.getRight(), y, 1.f);
        currentDb += scaleDivision;
    }
    
    
    // Draw frequency marks
    g.setColour(juce::Colours::lightgrey);
    
    std::vector<float> freqs
       {
           20, 50, 100,
           200, 500, 1000,
           2000, 5000, 10000,
           20000
       };
    
    auto generateLabel = [](float frequency)
    {
        bool addK = frequency >= 1000.f;
        auto label = juce::String(addK ? frequency/1000.f : frequency, 0);
        return label + (addK ? "k" : "");
    };
    
    auto drawLabel = [this, &g](juce::String text, float x)
    {
        g.drawSingleLineText(text, x, fftBoundingBox.getY() + getTextHeight());
    };
    
 
    
    drawLabel(generateLabel(freqs[0]) + "Hz", fftBoundingBox.getX());
    drawLabel(generateLabel(freqs.back()), fftBoundingBox.getX() + fftBoundingBox.getWidth() - getTextWidth());
    
    for(size_t i = 1; i < freqs.size() - 1; ++i)
    {
        auto scalePos = juce::mapFromLog10(freqs[i], freqs[0], freqs.back());
        auto x = fftBoundingBox.getWidth() * scalePos + fftBoundingBox.getX();
        g.drawLine(x, fftBoundingBox.getY(), x, fftBoundingBox.getBottom(), 1.f);
        drawLabel(generateLabel(freqs[i]), x + getTextWidth() / 4.0f);  //added a bit of space
    }
    
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
