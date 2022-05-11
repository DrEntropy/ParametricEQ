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
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::lightblue);
        
        if(isShowingAsOn())
        {
            g.setColour(juce::Colours::green);
        }
        
        juce::Path powerButton;
        
        auto size = juce::jmin(bounds.getWidth(),bounds.getHeight()) - 12;  // a bit shrinking
        auto r = bounds.withSizeKeepingCentre(size,size).toFloat();
        float ang = 30.f; //degrees
        size -= 6;
        powerButton.addCentredArc(r.getCentreX(), r.getCentreY(), size*0.5, size*0.5, 0.f, juce::degreesToRadians(ang), juce::degreesToRadians(360.f-ang), true);
            
        // line from center to top of box r
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
            
        juce::PathStrokeType pst(4, juce::PathStrokeType::curved);

        g.strokePath(powerButton, pst);
        g.drawRect(bounds, 4);
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
