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

 

struct TextSliderLookAndFeel : juce::LookAndFeel_V4
{
    
    
};

struct TextOnlyHorizontalSlider : juce::Slider
{
    TextOnlyHorizontalSlider() : juce::Slider(juce::Slider::SliderStyle::LinearHorizontal,
                                              juce::Slider::TextEntryBoxPosition::NoTextBox)
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
        setLookAndFeel(&textSliderLookAndFeel);
        
        addAndMakeVisible(frequencySlider);

        
        addAndMakeVisible(qSlider);
        
      
        
        if(isCut)
            gainOrSlopeSlider.reset(new SlopeSlider());
        else
            gainOrSlopeSlider.reset(new GainSlider());
        
        addAndMakeVisible(*gainOrSlopeSlider);
        
        AttachSliders(Channel::Left);
        

    }
    

    ~EQParamWidget() override
    {
        setLookAndFeel(nullptr);
    }
   
    void AttachSliders(Channel channel)
    {
        frequencyAttachment.reset(new SliderAttachment(apvts, createFreqParamString(channel, filterNumber), frequencySlider));
        qAttachment.reset(new SliderAttachment(apvts, createQParamString(channel, filterNumber), qSlider));
        
        juce::String gainOrSlopeParamString;
        
        if(isCut)
            gainOrSlopeParamString = createSlopeParamString(channel, filterNumber);
        else
            gainOrSlopeParamString = createGainParamString(channel, filterNumber);
        
        gainOrSlopeAttachment.reset(new SliderAttachment(apvts, gainOrSlopeParamString, *gainOrSlopeSlider));
    }

    void paint (juce::Graphics& g) override
    {

        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(juce::Colours::lightgrey);
        g.drawRect(getLocalBounds(), 2);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto height = bounds.getHeight();
        frequencySlider.setBounds(bounds.removeFromTop(height/4));
        qSlider.setBounds(bounds.removeFromTop(height/4));
        gainOrSlopeSlider->setBounds(bounds.removeFromTop(height/4));
 
    }

private:
    
    int filterNumber;
    bool isCut;
    
    juce::AudioProcessorValueTreeState& apvts;
    HertzSlider frequencySlider;
    QualitySlider qSlider;
    
    std::unique_ptr<juce::Slider> gainOrSlopeSlider;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    std::unique_ptr<SliderAttachment> frequencyAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainOrSlopeAttachment;
    
    TextSliderLookAndFeel textSliderLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamWidget)
};
