/*
  ==============================================================================

    GlobalBypass.h
    Created: 10 May 2022 2:01:22pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class GlobalBypass : public juce::Button, juce::Timer
{
public:
    GlobalBypass(ParametricEQAudioProcessor& proc) : Button(""), processor (proc)
    {
        setClickingTogglesState(true);
        setToggleState(true, juce::NotificationType::dontSendNotification);
        startTimerHz(60);
    }
    

    ~GlobalBypass() override = default;
    
    void paintButton (juce::Graphics& g,bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // this is all just placeholder
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::blue);
        if(isShowingAsOn())
        {
            g.setColour(juce::Colours::green);
        }
        
        g.fillRect(bounds.reduced(4));
        g.setColour(juce::Colours::darkblue);
        g.drawRect(bounds, 2);
    }
    
    void timerCallback() override
    {
        bool atLeastOneOn = processor.isAnyActiveOn();
        if(!atLeastOneOn && isShowingAsOn())
        {
            setToggleState(true, juce::NotificationType::dontSendNotification);
            DBG("All off");
        }
       
        if(atLeastOneOn && ! isShowingAsOn())
        {
            setToggleState(false, juce::NotificationType::dontSendNotification);
            DBG("at lease one  on");
        }
    }

protected:
    void clicked() override
    {
        processor.setBypassed(! isShowingAsOn());
    }
    
private:
    
    bool isShowingAsOn()
    {
        return !getToggleState();
    }
    ParametricEQAudioProcessor& processor;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalBypass)
};
