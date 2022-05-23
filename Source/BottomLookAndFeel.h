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
        
        g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::black);
        
        g.fillRect(bounds);
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1);
        
    }
    
    
    void drawRotarySlider(juce::Graphics & g, int x,int y, int width, int height, float sliderPosProportional,
                                       float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) override
    {
        auto bounds = juce::Rectangle<float> {static_cast<float>(x),static_cast<float>(y),
                                      static_cast<float>(width),static_cast<float>(height)};
        
        auto extraWidth = bounds.getWidth()  - bounds.getHeight();
        
        constexpr float dialWidth{2.f};
        
        if(extraWidth > 0)
        {
            bounds.reduce(extraWidth / 2.f, 0);
        }
        
        auto enabled = slider.isEnabled();
        
        
        g.setColour( enabled ? juce::Colours::lightblue : juce::Colours::darkgrey);
        g.fillEllipse(bounds);
        
        g.setColour(enabled ? juce::Colours::white : juce::Colours::lightgrey);
        g.drawEllipse(bounds, 1.0f);
        
 
            
        auto center = bounds.getCentre();
        juce::Path p;
        
        juce::Rectangle<float> r;
        // the pointer
        r.setLeft(center.getX() - dialWidth);
        r.setRight(center.getX() + dialWidth);
        r.setTop(bounds.getY());
        r.setBottom(center.getY());
        p.addRoundedRectangle(r, dialWidth);
            
            
        jassert(rotaryStartAngle<rotaryEndAngle);
            
        auto sliderAngle = juce::jmap(sliderPosProportional, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);
            
        p.applyTransform(juce::AffineTransform().rotation(sliderAngle, center.getX(), center.getY()));
            
        g.fillPath(p); // draw the dial indicator
    }
    
    
};

