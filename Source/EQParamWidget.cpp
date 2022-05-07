/*
  ==============================================================================

    EQParamWidget.cpp
    Created: 6 May 2022 3:35:25pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "EQParamWidget.h"


EQParamWidget::EQParamWidget(juce::AudioProcessorValueTreeState& apvts, int filterNumber, bool isCut) : apvts (apvts), filterNumber (filterNumber), isCut (isCut)
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


EQParamWidget::~EQParamWidget() 
{
    setLookAndFeel(nullptr);
}

void EQParamWidget::setProcessingMode(ChannelMode mode)
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
        leftMidButton.setToggleState(true,juce::NotificationType::sendNotification);
    }
}

void EQParamWidget::setUpButton(juce::Button& button)
{
    addChildComponent(button);
    button.setRadioGroupId(1);
    button.setClickingTogglesState(true);
}

void EQParamWidget::attachSliders(Channel channel)
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

void EQParamWidget::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void EQParamWidget::resized()  
{
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();
   
    frequencySlider.setBounds(bounds.removeFromTop(height/4));
    qSlider.setBounds(bounds.removeFromTop(height/4));
    gainOrSlopeSlider->setBounds(bounds.removeFromTop(height/4));
    
    auto buttonHeight = bounds.getHeight();
    
    auto leftButtonBounds = bounds.removeFromLeft(bounds.getWidth()/2).reduced(BUTTON_MARGIN);
    leftButtonBounds.setX(leftButtonBounds.getX()+leftButtonBounds.getWidth()-buttonHeight);
    leftButtonBounds.setWidth(buttonHeight);
    auto rightButtonBounds = bounds.reduced(BUTTON_MARGIN).withWidth(buttonHeight);
    leftMidButton.setBounds(leftButtonBounds);
    rightSideButton.setBounds(rightButtonBounds);
}