/*
  ==============================================================================

    AnalyzerWidgets.cpp
    Created: 30 May 2022 10:15:32am
    Author:  Ronald Legere

  ==============================================================================
*/

#include "AnalyzerWidgets.h"
#include "ChainHelpers.h"


#define BAND_OPACITY_SELECTED 0.5f
#define BAND_OPACITY_NOT_SELECTED 0.0f
#define BASE_HUE_RIGHT 0.5f
#define MAX_HUE_RIGHT 0.7f
#define BASE_HUE_LEFT 0.1f
#define MAX_HUE_LEFT 0.4f
#define BAND_SATURATION 0.8f

juce::Colour getColour(ChainPosition cp, Channel ch)
{
    float band0To1 =   static_cast<float> (cp) / (ChainHelpers::numberOfBands - 1);
    switch(ch)
    {
        case Channel::Left:
            return juce::Colour::fromHSV(juce::jmap(band0To1, BASE_HUE_LEFT, MAX_HUE_LEFT) , BAND_SATURATION, 1.0f, 1.0f);
        case Channel::Right:
            return juce::Colour::fromHSV(juce::jmap(band0To1, BASE_HUE_RIGHT, MAX_HUE_RIGHT), BAND_SATURATION, 1.0f, 1.0f);
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




 
