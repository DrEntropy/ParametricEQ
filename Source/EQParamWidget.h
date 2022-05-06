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

#define SLIDER_BORDER 2
#define BUTTON_MARGIN 3
#define BUTTONS_SIDE_MARGIN 5



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

            g.setColour(juce::Colours::white);
            g.drawRect(bounds, SLIDER_BORDER);
            bounds.reduce(SLIDER_BORDER, SLIDER_BORDER);
            
            // rescale slider position
            sliderPos = juce::jmap(sliderPos, static_cast<float>(x), static_cast<float>(x) + width, bounds.getX(), bounds.getWidth());
            
            g.setColour(juce::Colours::black);
            auto barBounds = bounds.withWidth(sliderPos);
            g.fillRect(barBounds);
            
            g.setColour(juce::Colours::white);
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
    EQParamWidget(juce::AudioProcessorValueTreeState& apvts, int filterNumber, bool isCut) : apvts (apvts), filterNumber (filterNumber), isCut (isCut)
    {
        setLookAndFeel(&eQParamLookAndFeel);
        
        addAndMakeVisible(frequencySlider);
        addAndMakeVisible(qSlider);
      
        if(isCut)
            gainOrSlopeSlider.reset(new SlopeSlider());
        else
            gainOrSlopeSlider.reset(new GainSlider());
        
        addAndMakeVisible(*gainOrSlopeSlider);
        
        attachSliders(Channel::Left);
        
        setUpButton(leftMidButton);
        setUpButton(rightSideButton);
        leftMidButton.setToggleState(true, juce::NotificationType::dontSendNotification);
        
        auto safePtr = juce::Component::SafePointer<EQParamWidget>(this);
        leftMidButton.onClick = [safePtr]()
        {
            if(auto* comp = safePtr.getComponent() )
                comp->attachSliders(Channel::Left);
        };
        rightSideButton.onClick = [safePtr]()
        {
            if(auto* comp = safePtr.getComponent() )
                comp->attachSliders(Channel::Right);
        };
        
 
        ChannelMode mode = static_cast<ChannelMode>(apvts.getRawParameterValue("Processing Mode")->load());
        setProcessingMode(mode);
        
        modeListener.reset(new ParamListener(apvts.getParameter("Processing Mode"),
                                            [safePtr](float v)
                                             {
                                              if(auto* comp = safePtr.getComponent() )
                                                  comp->setProcessingMode(static_cast<ChannelMode>(v));
                                             }));
    }
    

    ~EQParamWidget() override
    {
        setLookAndFeel(nullptr);
    }
    
    void setProcessingMode(ChannelMode mode)
    {
        if(mode != ChannelMode::DualMono)
        {
            leftMidButton.setVisible(true);
            leftMidButton.setButtonText(mode == ChannelMode::Stereo ? "L" : "M");
            rightSideButton.setVisible(true);
            rightSideButton.setButtonText(mode == ChannelMode::Stereo ? "R" : "S");
        }
        else
        {
            leftMidButton.setVisible(false);
            rightSideButton.setVisible(false);
        }
    }
    
    void setUpButton(juce::Button& button)
    {
        addChildComponent(button);
        button.setRadioGroupId(1);
        button.setClickingTogglesState(true);
    }
   
    void attachSliders(Channel channel)
    {
        frequencyAttachment.reset(); //must first delete old attachment before creating new one!
        frequencyAttachment.reset(new SliderAttachment(apvts, createFreqParamString(channel, filterNumber), frequencySlider));
        
        qAttachment.reset();
        qAttachment.reset(new SliderAttachment(apvts, createQParamString(channel, filterNumber), qSlider));
        
        juce::String gainOrSlopeParamString;
        
        if(isCut)
            gainOrSlopeParamString = createSlopeParamString(channel, filterNumber);
        else
            gainOrSlopeParamString = createGainParamString(channel, filterNumber);
        
        gainOrSlopeAttachment.reset();
        gainOrSlopeAttachment.reset(new SliderAttachment(apvts, gainOrSlopeParamString, *gainOrSlopeSlider));
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto height = bounds.getHeight();
       
        frequencySlider.setBounds(bounds.removeFromTop(height/4));
        qSlider.setBounds(bounds.removeFromTop(height/4));
        gainOrSlopeSlider->setBounds(bounds.removeFromTop(height/4));
        
        bounds.reduce(BUTTONS_SIDE_MARGIN, 0);
        leftMidButton.setBounds(bounds.removeFromLeft(bounds.getWidth()/2).reduced(BUTTON_MARGIN));
        rightSideButton.setBounds(bounds.reduced(BUTTON_MARGIN));
    }
    

private:
    
    int filterNumber;
    bool isCut;
    
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
