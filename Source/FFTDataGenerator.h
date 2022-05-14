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

#define FFT_FIFO_DEPTH 100

enum class FFTOrder
{
    FFT2048 = 11,
    FFT4096 = 12,
    FFT8192 = 13
};

struct FFTDataGenerator
{
    /*
     produces the FFT data from an audio buffer.
     */
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData);
    
    void changeOrder(FFTOrder order);
    
    size_t getFFTSize() const { return 1 << static_cast<int>(order); }
    
    size_t getNumAvailableFFTDataBlocks() const;
    bool getFFTData(std::vector<float>&& fftData);
private:
    FFTOrder order;
    std::vector<float> fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<std::vector<float>, FFT_FIFO_DEPTH> fftDataFifo;
};
