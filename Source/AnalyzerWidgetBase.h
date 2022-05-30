/*
  ==============================================================================

    AnalyzerWidgetBase.h
    Created: 30 May 2022 10:08:36am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class AnalyzerWidgetBase  : public juce::Component
{
public:
    AnalyzerWidgetBase();
    ~AnalyzerWidgetBase() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyzerWidgetBase)
};
