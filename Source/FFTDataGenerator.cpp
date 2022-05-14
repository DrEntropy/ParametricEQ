/*
  ==============================================================================

    FFTDataGenerator.cpp
    Created: 13 May 2022 2:22:28pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "FFTDataGenerator.h"
#include <algorithm>


void FFTDataGenerator::produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData)
{
    auto size = getFFTSize();
    
    jassert(size == audioData.getNumSamples());
    
    fftData.clear();
    fftData.resize(2 * size,0.f);
    
 
    auto readPointer = audioData.getReadPointer(0);  //single channel

    // copy all of the samples from audioData into fftData, first half
    std::copy(readPointer, readPointer+size, fftData.begin());
    
    // apply the window to fftData
    window->multiplyWithWindowingTable(fftData.data(), size);

    // perform the forwardFFT frequencyOnlyForwardTransform on fftData
    forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());

    //  next, normalize the fft values in fftData. this is accomplished by dividing every index by the number of bins
    // the number of bins is HALF the fftSize and convert every index in fftData to Decibels, using juce::Decibels::gainToDecibels
    for(auto& value : fftData)
        value = juce::Decibels::gainToDecibels(2.0 * value / size);
  

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
    // the fftData, filled with zeros (again?)
    fftData.clear();
    fftData.resize(2 * size,0.f);
    
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
