/*
  ==============================================================================

    DbScale.h
    Created: 28 Apr 2022 2:39:51pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
 
struct Tick
{
    float db { 0.f };
    int y { 0 };
};

class DbScale  : public juce::Component
{
public:
    DbScale(){}
    ~DbScale() override = default;
    void paint (juce::Graphics&) override;
    void buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb);
    static std::vector<Tick> getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb);
    
private:
    juce::Image bkgd;
    const int textHeight = 14;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DbScale)
};
