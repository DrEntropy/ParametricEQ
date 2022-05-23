/*
  ==============================================================================

    SwitchSlider.h
    Created: 22 May 2022 11:08:18pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SwitchSlider  : public juce::Slider
{
public:
 

    SwitchSlider() : juce::Slider(juce::Slider::SliderStyle::LinearVertical,
                                  juce::Slider::TextEntryBoxPosition::NoTextBox){}
    
    ~SwitchSlider() override = default;
   

    void paint (juce::Graphics& g) override
    {
        auto enabled = isEnabled();
        auto bounds = getLocalBounds();
        
        auto sliderRect = bounds.removeFromLeft(bounds.getWidth() / sliderWidthDiv);
        
        auto startX = sliderRect.getRight() + textHeight;
        auto startY = sliderRect.getBottom() - sliderRect.getWidth() / 2;
        auto width = getLocalBounds().getRight() - startX;
        
        auto deltaY =  sliderRect.getHeight() - sliderRect.getWidth();
        
        if(choices.size() > 1)
            deltaY /= (choices.size() - 1);
        
        // g.setColour(enabled ? juce::Colours::white : juce::Colours::grey);
        
        auto value = getValue();
        
        for(int i = 0; i < choices.size(); ++i)
        {
           if(enabled && static_cast<int>(value) == i)
               g.setColour(juce::Colours::green);
           else if(enabled)
               g.setColour(juce::Colours::white);
           else
               g.setColour(juce::Colours::grey);
               
           g.drawFittedText(choices[i], startX, startY - textHeight / 2, width, textHeight, juce::Justification::centredLeft , 1);
           startY -= deltaY;
        }
        
        auto range = getRange();
        
        
        auto getPos{
            [&range, &sliderRect](double v)
            {
                return juce::jmap(v, range.getStart(), range.getEnd(), sliderRect.toDouble().getBottom(), sliderRect.toDouble().getY());
            }
        };
        
        getLookAndFeel().drawLinearSlider (g,
                             sliderRect.getX(), sliderRect.getY(),
                             sliderRect.getWidth(), sliderRect.getHeight(),
                             getPos (value),
                             getPos (getMinimum()),
                             getPos (getMaximum()),
                             juce::Slider::SliderStyle::LinearVertical, *this);
     
    }

    juce::StringArray choices;
    
private:
    
    static constexpr int textHeight = 14;
    static constexpr int sliderWidthDiv = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwitchSlider)
};
