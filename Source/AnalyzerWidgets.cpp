/*
  ==============================================================================

    AnalyzerWidgets.cpp
    Created: 30 May 2022 10:15:32am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "AnalyzerWidgets.h"


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
    // paint rectangle
}




 
