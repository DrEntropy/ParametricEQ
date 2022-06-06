/*
  ==============================================================================

    AnalyzerWidgets.cpp
    Created: 30 May 2022 10:15:32am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "AnalyzerWidgets.h"

#define BAND_OPACITY_SELECTED 0.5
#define BAND_OPACITY_NOT_SELECTED 0.25

juce::Colour getColour(ChainPosition cp, Channel ch)
{
    //TODO paint different shade for each cp and different colour for each channel
    return juce::Colours::lime;
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
    g.drawRect(getLocalBounds().toFloat());
}




 
