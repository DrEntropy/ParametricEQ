/*
  ==============================================================================

    SampleRateListener.h
    Created: 17 May 2022 12:09:19pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

struct SampleRateListener
{
    virtual ~SampleRateListener() = default;
    virtual void sampleRateChanged(double sr) = 0;
};
