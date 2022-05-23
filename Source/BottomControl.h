/*
  ==============================================================================

    BottomControl.h
    Created: 22 May 2022 3:51:55pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
 
#include "SwitchSlider.h"

//==============================================================================
/*
*/
template <typename SliderType>
class BottomControl  : public juce::Component
{
public:
    BottomControl(juce::AudioProcessorValueTreeState& apvts, juce::String parameterName)
    {
        attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, parameterName, slider));
        
        
        auto myParameter = dynamic_cast<juce::AudioParameterChoice*>( apvts.getParameter(parameterName) );
        
        if constexpr (std::is_same<SliderType, SwitchSlider>::value)
        {
            if(myParameter)
                slider.choices =  myParameter->choices;
        }
        
        addAndMakeVisible(slider);
        label.setJustificationType(juce::Justification::centred);
        label.setText(apvts.getParameter(parameterName)->name, juce::NotificationType::dontSendNotification);
        addAndMakeVisible(label);
    }
    
    ~BottomControl() override
    {
        attachment.reset();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto w = bounds.getWidth();
        bounds.removeFromLeft(w / marginDiv);
        label.setBounds(bounds.removeFromTop(w / labelHeightDiv));
        bounds.removeFromBottom(w / marginDiv);
        
        slider.setBounds(bounds);
    }


private:
    
    SliderType slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    juce::Label label;
    
    static constexpr int marginDiv {10};
    static constexpr int labelHeightDiv {4};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomControl)
};
