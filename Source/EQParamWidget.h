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

 

struct EQParamLookAndFeel : juce::LookAndFeel_V4
{
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float minSliderPos, float maxSliderPos,
                                              const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        // only override LinearHorizontal (but why? This is bar already. Also this doesnt work because the drawLinearSlider call for LinearHorizontal adds
        // some padding for some reason.
        if(style != juce::Slider::SliderStyle::LinearHorizontal)
        {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos,
                                                           style, slider);
            return;
        }
    
        auto bounds = slider.getLocalBounds().toFloat();
        // rescale slider positions
        sliderPos = juce::jmap(sliderPos, static_cast<float> (x), static_cast<float> (x) + width, 0.f, bounds.getWidth());
        g.setColour (juce::Colours::black);
        g.fillRect (juce::Rectangle<float> (0.f, 0.f, sliderPos,  bounds.getHeight()));
       
    }
    
    
};

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
        return " Hz";
    }
};

struct QualitySlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        return "";
    }
};

struct SlopeSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        return " dB/oct";
    }
};

struct GainSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString()
    {
        return " dB";
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
        
        AttachSliders(Channel::Left);
        
        addChildComponent(leftMidButton);
        addChildComponent(rightSideButton);
    
        
        auto safePtr = juce::Component::SafePointer<EQParamWidget>(this);
        leftMidButton.onClick = [safePtr]()
        {
            if(auto* comp = safePtr.getComponent() )
                comp->AttachSliders(Channel::Left);
        };
        rightSideButton.onClick = [safePtr]()
        {
            if(auto* comp = safePtr.getComponent() )
                comp->AttachSliders(Channel::Right);
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
            rightSideButton.setVisible(true);
        }
        else
        {
            leftMidButton.setVisible(false);
            rightSideButton.setVisible(false);
        }
    }
   
    void AttachSliders(Channel channel)
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
        g.setColour(juce::Colours::blue);
        g.drawRect(getLocalBounds(), 2);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.reduce(2,2);
        auto height = bounds.getHeight();
        auto width = bounds.getWidth();
        frequencySlider.setBounds(bounds.removeFromTop(height/4));
        qSlider.setBounds(bounds.removeFromTop(height/4));
        gainOrSlopeSlider->setBounds(bounds.removeFromTop(height/4));
        leftMidButton.setBounds(bounds.removeFromLeft(width/2));
        rightSideButton.setBounds(bounds);
 
    }
    

private:
    
    int filterNumber;
    bool isCut;
    
    juce::AudioProcessorValueTreeState& apvts;
    HertzSlider frequencySlider;
    QualitySlider qSlider;
    std::unique_ptr<juce::Slider> gainOrSlopeSlider;
    juce::TextButton leftMidButton, rightSideButton;
    
    // TODO add ParamListeneres for  bypass
    std::unique_ptr<ParamListener> modeListener;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    std::unique_ptr<SliderAttachment> frequencyAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainOrSlopeAttachment;
    
    EQParamLookAndFeel eQParamLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamWidget)
};