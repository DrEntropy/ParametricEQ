/*
  ==============================================================================

    BottomLookAandFeel.h
    Created: 22 May 2022 3:25:10pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


struct BottomLookAndFeel : juce::LookAndFeel_V4
{
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float minSliderPos, float maxSliderPos,
                                              const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        
        if(style == juce::Slider::SliderStyle::LinearVertical)
        {
            auto bounds = slider.getLocalBounds().toFloat();
            bool enabled = slider.isEnabled();
            
            g.setColour(enabled ? juce::Colours::white : juce::Colours::grey);
            g.drawRect(x, y, width, height);
 
            auto knobHeight = static_cast<float>(width);
            
            // rescale slider position to include knob height
            auto sliderY = juce::jmap(sliderPos, static_cast<float>(y), static_cast<float>(y + height),
                                      static_cast<float>(y), static_cast<float>(y + height - knobHeight));
            
            g.setColour(enabled ? juce::Colours::lightblue : juce::Colours::grey);
            g.fillRect(static_cast<float>(x), sliderY, knobHeight, knobHeight);
      
        }
        else
        {
        // draw a normal slider in other cases.
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
    
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds();
        if(button.getToggleState())
            g.setColour(juce::Colours::green);
        else
            g.setColour(juce::Colours::black);
        
        g.fillRect(bounds);
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1);
        
    }
    
    
};

