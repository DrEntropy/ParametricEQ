/*
  ==============================================================================

    FFTDataGenerator.h
    Created: 13 May 2022 2:22:28pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <vector>
#include <JuceHeader.h>
#include "Fifo.h"
#include "AnalyzerProperties.h"

#define FFT_FIFO_DEPTH 100



struct FFTDataGenerator
{
    /*
     produces the FFT data from an audio buffer.
     */
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData);
    
    void changeOrder(AnalyzerProperties::FFTOrder order);
    
    size_t getFFTSize() const { return 1 << static_cast<int>(order); }
    
    size_t getNumAvailableFFTDataBlocks() const;
    bool getFFTData(std::vector<float>& fftData);
    
private:
    AnalyzerProperties::FFTOrder order;
    std::vector<float> fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<std::vector<float>, FFT_FIFO_DEPTH> fftDataFifo;
};
