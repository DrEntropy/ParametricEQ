/*
  ==============================================================================

    EQParamWidget.h
    Created: 5 May 2022 1:22:28pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// helper classes

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
    EQParamWidget(int filterNumber, bool isCut)
    {
        setLookAndFeel(&textSliderLookAndFeel);
        addAndMakeVisible(frequencySlider);
        addAndMakeVisible(qSlider);
        //gainOrSlopeSlider.reset(make_unique<SlopeSlider>());
        //addAndMakeVisible(*gainOrSlopeSlider);
        

    }

    ~EQParamWidget() override
    {
        setLookAndFeel(nullptr);
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
 
    }

private:
    HertzSlider frequencySlider;
    QualitySlider qSlider;
    
    std::unique_ptr<juce::Slider> gainOrSlopeSlider;
    
    TextSliderLookAndFeel textSliderLookAndFeel;
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQParamWidget)
};
