/*
  ==============================================================================

    PathProducer.cpp
    Created: 16 May 2022 9:49:48am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "PathProducer.h"

template<typename BlockType>
PathProducer<BlockType>::PathProducer(double sr, SingleChannelSampleFifo<BlockType>&)
{
    // do stuff
}


template<typename BlockType>
PathProducer<BlockType>::~PathProducer()
{
    //
}

template<typename BlockType>
void PathProducer<BlockType>::run()
{
    
}

template<typename BlockType>
void PathProducer<BlockType>::changeOrder(FFTOrder o)
{
    
}

template<typename BlockType>
int PathProducer<BlockType>::getFFTSize() const
{
    
}

template<typename BlockType>
double PathProducer<BlockType>::getBinWidth() const
{
    
}

template<typename BlockType>
void PathProducer<BlockType>::pauseThread()
{
    
}

template<typename BlockType>
void PathProducer<BlockType>::setFFTRectBounds(juce::Rectangle<float>)
{
    
}

template<typename BlockType>
void PathProducer<BlockType>::setDecayRate(float dr)
{
    
}

template<typename BlockType>
bool PathProducer<BlockType>::pull(juce::Path&&)
{
    
}

template<typename BlockType>
int PathProducer<BlockType>::getNumAvailableForReading() const
{
    
}

template<typename BlockType>
void PathProducer<BlockType>::toggleProcessing(bool)
{
    
}

template<typename BlockType>
void PathProducer<BlockType>::changePathRange(float negativeInfinityDb, float maxDb)
{
    
}
