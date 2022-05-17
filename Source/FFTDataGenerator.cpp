/*
  ==============================================================================

    FFTDataGenerator.cpp
    Created: 13 May 2022 2:22:28pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "FFTDataGenerator.h"
#include "EQConstants.h"


void FFTDataGenerator::produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData)
{
    auto size = getFFTSize();
    
    jassert(size == audioData.getNumSamples());
    
    auto readPointer = audioData.getReadPointer(0);  //single channel

    // copy all of the samples from audioData into fftData, first half
    std::copy(readPointer, readPointer+size, fftData.begin());
    
    // apply the window to fftData
    window->multiplyWithWindowingTable(fftData.data(), size);

    // perform the forwardFFT frequencyOnlyForwardTransform on fftData
    forwardFFT->performFrequencyOnlyForwardTransform(fftData.data(), true);

    // normalize
    auto numBins = size / 2;
    juce::FloatVectorOperations::multiply(fftData.data(), 1.0f / numBins, numBins + 1);
    
    // DC and nyquist are both frequency components so have different normalization.
    fftData[0] *= 2.f;
    fftData[numBins] *= 2.f;
    
    // convert to dB
    
    for( size_t i = 0; i <= numBins; ++i )
     {
         fftData[i] = juce::Decibels::gainToDecibels(fftData[i], NEGATIVE_INFINITY);
     }
    
  
    // finally, push fftData into the Fifo for others to consume.
    fftDataFifo.push(fftData);
}


void FFTDataGenerator::changeOrder(FFTOrder newOrder)
{
    order = newOrder;
    auto size = getFFTSize();
    // the window  use blackmanHarris for the windowing function
    window = std::make_unique<juce::dsp::WindowingFunction<float>>(size, juce::dsp::WindowingFunction<float>::blackmanHarris);
    // the forwardFFT
    forwardFFT = std::make_unique<juce::dsp::FFT>(static_cast<int>(order));
    // the fftData, filled with zeros
    fftData.assign(2 * size, 0.f);
    
    fftDataFifo.prepare(2 * size);
}

size_t FFTDataGenerator::getNumAvailableFFTDataBlocks() const
{
   return fftDataFifo.getNumAvailableForReading();
}

bool FFTDataGenerator::getFFTData(std::vector<float>&& fftData)
{
   return fftDataFifo.exchange(std::move(fftData));
}
