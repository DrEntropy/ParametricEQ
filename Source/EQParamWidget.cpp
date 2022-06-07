/*
  ==============================================================================

    EQParamWidget.cpp
    Created: 6 May 2022 3:35:25pm
    Author:  Ronald Legere

  ==============================================================================
*/

#include "EQParamWidget.h"


EQParamWidget::EQParamWidget(juce::AudioProcessorValueTreeState& apvts, ChainPosition cp, bool isCut) : apvts (apvts), chainPos(cp), isCut (isCut)
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
    refreshButtons(mode);
    
    modeListener.reset(new ParamListener(apvts.getParameter("Processing Mode"),
                                        [safePtr](float v)
                                         {
                                          if(auto* comp = safePtr.getComponent() )
                                              comp->refreshButtons(static_cast<ChannelMode>(v));
                                         }));
    
    auto leftBypass = apvts.getParameter(createBypassParamString(Channel::Left, cp));
    auto rightBypass = apvts.getParameter(createBypassParamString(Channel::Right, cp));
    
    leftMidBypassListener.reset(new ParamListener(leftBypass,
                                                  [safePtr](float v)
                                                   {
                                                    if(auto* comp = safePtr.getComponent() )
                                                        comp->refreshSliders(Channel::Left);
                                                   }));
    
    rightSideBypassListener.reset(new ParamListener(rightBypass,
                                                     [safePtr](float v)
                                                      {
                                                       if(auto* comp = safePtr.getComponent() )
                                                           comp->refreshSliders(Channel::Right);
                                                      }));
    
    activeChannel = Channel::Left;
    refreshSliders(activeChannel);
   
}


EQParamWidget::~EQParamWidget() 
{
    setLookAndFeel(nullptr);
}

void EQParamWidget::refreshSliders(Channel ch)
{
   
    if(ch != activeChannel)
        return;
    
    auto bypass = apvts.getParameter(createBypassParamString(activeChannel, chainPos));
    auto bypassed  = bypass->getValue() > 0.5;
    auto colour = bypassed ? juce::Colours::grey : (selected ? juce::Colours::lightgreen : juce::Colours::white);

    gainOrSlopeSlider->setColour(juce::Slider::textBoxOutlineColourId, colour);
    qSlider.setColour(juce::Slider::textBoxOutlineColourId, colour);
    frequencySlider.setColour(juce::Slider::textBoxOutlineColourId, colour);
}

void EQParamWidget::refreshButtons(ChannelMode mode)
{
    if(mode != ChannelMode::Stereo)
    {
        leftMidButton.setVisible(true);
        leftMidButton.setButtonText(mode == ChannelMode::DualMono ? "L" : "M");
        rightSideButton.setVisible(true);
        rightSideButton.setButtonText(mode == ChannelMode::DualMono ? "R" : "S");
    }
    else
    {
        leftMidButton.setVisible(false);
        rightSideButton.setVisible(false);
        leftMidButton.setToggleState(true,juce::NotificationType::sendNotification);
        activeChannel = Channel::Left;
    }
}


//TODO test this crap out!
void EQParamWidget::bandSelected(Channel ch)
{
    selected = true;
    if(activeChannel == ch)
    {
        refreshSliders(ch);
    }
    else  // push the other button
    {
        activeChannel = ch;
        if(ch == Channel::Left)
            leftMidButton.setToggleState(true, juce::NotificationType::sendNotification);
        else
            rightSideButton.setToggleState(true, juce::NotificationType::sendNotification);
        
    }
        
}

void EQParamWidget::bandCleared()
{
    selected = false;
    refreshSliders(activeChannel);
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
    frequencyAttachment.reset(new SliderAttachment(apvts, createFreqParamString(channel, chainPos), frequencySlider));

    qAttachment.reset();
    qAttachment.reset(new SliderAttachment(apvts, createQParamString(channel, chainPos), qSlider));

    juce::String gainOrSlopeParamString;

    if(isCut)
        gainOrSlopeParamString = createSlopeParamString(channel, chainPos);
    else
        gainOrSlopeParamString = createGainParamString(channel, chainPos);

    gainOrSlopeAttachment.reset();
    gainOrSlopeAttachment.reset(new SliderAttachment(apvts, gainOrSlopeParamString, *gainOrSlopeSlider));
    
    activeChannel = channel;
    refreshSliders(activeChannel);
}

 

void EQParamWidget::paintOverChildren(juce::Graphics& g)
{
    auto bounds = getLocalBounds();


    g.setColour(juce::Colours::white);

    g.fillRect(bounds.removeFromTop(HALF_SLIDER_BORDER));  // rest of top line, made room for this in resized.
    
    auto x = bounds.getX();
    auto y = bounds.getY();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    auto bottomY = y + 3 * (height / 4);
    
    g.drawLine(x, bottomY, x + width, bottomY, HALF_SLIDER_BORDER * 2);  // bottom line
}



void EQParamWidget::resized()  
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(HALF_SLIDER_BORDER); // make room for thickening top line
    
    auto height = bounds.getHeight();
    frequencySlider.setBounds(bounds.removeFromTop(height / 4));
    qSlider.setBounds(bounds.removeFromTop(height / 4));
    gainOrSlopeSlider->setBounds(bounds.removeFromTop(height / 4));
    
    auto buttonHeight = bounds.getHeight();
    
    auto leftButtonBounds = bounds.removeFromLeft(bounds.getWidth() / 2).reduced(BUTTON_MARGIN);
    leftButtonBounds.setX(leftButtonBounds.getX() + leftButtonBounds.getWidth() - buttonHeight);
    leftButtonBounds.setWidth(buttonHeight);
    
    auto rightButtonBounds = bounds.reduced(BUTTON_MARGIN).withWidth(buttonHeight);
    leftMidButton.setBounds(leftButtonBounds);
    rightSideButton.setBounds(rightButtonBounds);
}
