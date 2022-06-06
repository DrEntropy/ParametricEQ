/*
  ==============================================================================

    EQParamWidget.h
    Created: 5 May 2022 1:22:28pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ParameterHelpers.h"
#include "ParamListener.h"
#include "FilterInfo.h"

#define HALF_SLIDER_BORDER 1
#define BUTTON_MARGIN 3



struct TextOnlyHorizontalSlider : juce::Slider
{
    TextOnlyHorizontalSlider() : juce::Slider(juce::Slider::SliderStyle::LinearHorizontal,
                                              juce::Slider::TextEntryBoxPosition::NoTextBox )
    {
        /*
         setting this to false prevents the slider from snapping its value to wherever you click inside the slider bounds.
         */
        
        setSliderSnapsToMousePosition(false);
    }
    virtual ~TextOnlyHorizontalSlider() = default;
    virtual juce::String getDisplayString() = 0;
};

struct HertzSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        bool addK {false};
        auto freq = getValue();
        
        if(freq >= 1000.0f)
        {
            freq /= 1000.0f;
            addK = true;
        }
        
        auto freqString = juce::String(freq, (addK ? 2 : 0));
        return freqString + (addK ? "kHz" : "Hz");
    }
};

struct QualitySlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        return juce::String(getValue(), 2);
    }
};

struct SlopeSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        auto slope = static_cast<FilterInfo::Slope>(getValue());
        return FilterInfo::mapSlopeToString.at(slope);
    }
};

struct GainSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        return juce::String(getValue(), 2) + " dB";
    }
};


struct EQParamLookAndFeel : juce::LookAndFeel_V4
{
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float minSliderPos, float maxSliderPos,
                                              const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if(TextOnlyHorizontalSlider* textOnlyHSliderp = dynamic_cast<TextOnlyHorizontalSlider*> (&slider))
        {
            auto bounds = slider.getLocalBounds().toFloat();
            // Note i am using this outline colour for everything .
            auto mainColour = slider.findColour(juce::Slider::textBoxOutlineColourId);
            g.setColour(mainColour);
           
            g.drawRect(bounds, HALF_SLIDER_BORDER);
            bounds.reduce(HALF_SLIDER_BORDER, HALF_SLIDER_BORDER);
            
            // rescale slider position
            sliderPos = juce::jmap(sliderPos, static_cast<float>(x), static_cast<float>(x) + width, bounds.getX(), bounds.getWidth());
            
            g.setColour(juce::Colours::black);
            auto barBounds = bounds.withWidth(sliderPos);
            g.fillRect(barBounds);
            
            g.setColour(mainColour);
            juce::String message = textOnlyHSliderp->getDisplayString();
            g.drawFittedText(message, bounds.toNearestInt(), juce::Justification::centred, 1);
        }
        else
        {
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

class EQParamWidget  : public juce::Component
{
public:
    EQParamWidget(juce::AudioProcessorValueTreeState& apvts, ChainPosition chainPos, bool isCut);
    ~EQParamWidget() override;
    void refreshButtons(ChannelMode mode);
    void refreshSliders(bool bypassed, Channel ch);
    void setUpButton(juce::Button& button);
    void attachSliders(Channel channel);
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;
 

private:
    
    ChainPosition chainPos;
    bool isCut;
    
    Channel activeChannel;
    
    juce::AudioProcessorValueTreeState& apvts;
    HertzSlider frequencySlider;
    QualitySlider qSlider;
    std::unique_ptr<juce::Slider> gainOrSlopeSlider;
    juce::TextButton leftMidButton, rightSideButton;
    
    std::unique_ptr<ParamListener> modeListener;
    std::unique_ptr<ParamListener> leftMidBypassListener;
    std::unique_ptr<ParamListener> rightSideBypassListener;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    std::unique_ptr<SliderAttachment> frequencyAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainOrSlopeAttachment;
    
    
    EQParamLookAndFeel eQParamLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamWidget)
};
