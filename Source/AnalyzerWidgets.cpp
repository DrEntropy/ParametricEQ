/*
  ==============================================================================

    AnalyzerWidgets.cpp
    Created: 30 May 2022 10:15:32am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "AnalyzerWidgets.h"

#define BAND_OPACITY_SELECTED 0.5
#define BAND_OPACITY_NOT_SELECTED 0.1

juce::Colour getColour(ChainPosition cp, Channel ch)
{
    float colorMod =   static_cast<float> (cp) / 30.f;
    switch(ch)
    {
        case Channel::Left:
            return juce::Colour::fromHSV(0.2f + colorMod , 0.8f, 1.0f, 1.0f);
        case Channel::Right:
            return juce::Colour::fromHSV(0.5f + colorMod, 0.8f, 1.0f, 1.0f);
    }
  
}

AnalyzerWidgetBase::AnalyzerWidgetBase(ChainPosition cp, Channel ch) : chainPosition(cp), channel(ch){}

void AnalyzerWidgetBase::displayAsSelected(bool selected)
{
    isSelected = selected;
}

void AnalyzerWidgetBase::paint(juce::Graphics& g)
{
    if(isSelected)
        g.setColour(juce::Colours::red);
    else
        g.setColour(juce::Colours::blue);
    
    g.fillEllipse(getLocalBounds().toFloat());
}



AnalyzerBand::AnalyzerBand(ChainPosition cp, Channel ch) :  AnalyzerWidgetBase(cp, ch) {}

void AnalyzerBand::paint(juce::Graphics& g)
{
    
    g.setColour(getColour(chainPosition, channel));
    g.setOpacity(isSelected  ?  BAND_OPACITY_SELECTED : BAND_OPACITY_NOT_SELECTED);
    g.fillRect(getLocalBounds());
}




 
