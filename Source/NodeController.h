/*
  ==============================================================================

    NodeController.h
    Created: 30 May 2022 10:10:21am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include "AnalyzerBase.h"
#include "AnalyzerWidgets.h"
#include "AllParamsListener.h"
#include "ParameterAttachment.h"


static const double widthOctaves {std::log2(20000.0 / 20.0)};


struct BoundsContrainer : juce::ComponentBoundsConstrainer
{
    void  checkBounds (juce::Rectangle<int>& bounds, const juce::Rectangle<int>& /* old */, const juce::Rectangle<int>& /* limits */,
                       bool /* isStretchingTop */, bool /* isStretchingLeft */,
                       bool /* isStretchingBottom */, bool /* isStretchingRight */) override
    {
        auto centre = bounds.getCentre();
        
        if(centre.getY() < boundsLimit.getY())
            centre.setY( boundsLimit.getY());
        if(centre.getY() > boundsLimit.getBottom())
            centre.setY(boundsLimit.getBottom());
        if(centre.getX() >  boundsLimit.getRight())
            centre.setX(boundsLimit.getRight());
        if(centre.getX() < boundsLimit.getX())
            centre.setX(boundsLimit.getX());
        
        bounds.setCentre(centre);
    }
    
    juce::Rectangle<int> boundsLimit;
    
};

struct HorizontalConstrainer : juce::ComponentBoundsConstrainer
{
    // does not allow vertical movement.
    void  checkBounds (juce::Rectangle<int>& bounds, const juce::Rectangle<int>& old, const juce::Rectangle<int>& /* limits */,
                       bool /* isStretchingTop */, bool /* isStretchingLeft */,
                       bool /* isStretchingBottom */, bool /* isStretchingRight */) override
    {
        auto centre = bounds.getCentre();
    
        if(centre.getX() >  boundsLimit.getRight())
            centre.setX(boundsLimit.getRight());
        if(centre.getX() < boundsLimit.getX())
            centre.setX(boundsLimit.getX());
        
        centre.setY(old.getCentre().getY());
        
        bounds.setCentre(centre);
    }
    
    juce::Rectangle<int> boundsLimit;
    
};


struct NodeController : AnalyzerBase 
{
    NodeController(juce::AudioProcessorValueTreeState&);
    void resized() override;
    
    // Listeners
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;
    
    
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void bandMousedOver(ChainPosition cp, Channel ch) = 0;
        virtual void bandSelected(ChainPosition cp, Channel ch) = 0;
        virtual void clearSelection() = 0;
    };
    
    void addNodeListener (Listener*);
    void removeNodeListener (Listener*);
    
    // to be used for double click on controller and for the big button.
    void resetAllParameters();
    
private:
    void refreshWidgets();


    void updateNode(AnalyzerNode& node, juce::Rectangle<float> bBox);
    void updateBand(AnalyzerBand& band, juce::Rectangle<float> bBox);
    void activateQControls(ChainPosition pos, Channel ch);
    void deactivateQControls();
    
    void refreshQControls();
    
    void resetQ(ChainPosition cp, Channel ch);
    void resetFreq(ChainPosition cp, Channel ch);
    void resetGainOrSlope(ChainPosition cp, Channel ch);
    
    
    void debugMouse(juce::String type, const juce::MouseEvent &event);
    
    float frequencyFromX(float x);
    float gainFromY(float y);
    float slopeFromY(float y);
    
    template <typename Widget>
    void addWidget(size_t i, std::array<std::unique_ptr<Widget>, 16>& widgets, juce::String ID,
                   ChainPosition pos, Channel ch, bool alwaysOnTop = false)
    {
        widgets[i] = std::make_unique<Widget>(pos, ch);
        widgets[i]->setComponentID(ID + ":" + (ch == Channel::Left ? "L:" : "R:") + std::to_string(i));
        widgets[i]->addMouseListener(this, false);
        widgets[i]->setAlwaysOnTop(alwaysOnTop);
        addChildComponent(*widgets[i]);
    }
    
    AnalyzerNode* currentNode{nullptr};
    AnalyzerBand* currentBand{nullptr};
    
    std::array<std::unique_ptr<AnalyzerNode> , 16> nodes; //first 8 are left/mid, second  8 are right side.
    std::array<std::unique_ptr<AnalyzerBand> , 16> bands;
    
    AnalyzerQControl  qControlLeft{}, qControlRight{};
    
    inline bool qControlsVisible()
    {
        return qControlLeft.isVisible() || qControlRight.isVisible();
    }
    
    std::array<std::unique_ptr<ParameterAttachment>, 16> freqAttachements;
    std::array<std::unique_ptr<ParameterAttachment>, 16> qAttachements;
    std::array<std::unique_ptr<ParameterAttachment>, 16> gainOrSlopeAttachements;
    
    std::unique_ptr<AllParamsListener> allParamsListener;
    
    ParameterAttachment& getAttachmentForWidget(std::array<std::unique_ptr<ParameterAttachment>, 16>& attachments, AnalyzerWidgetBase* node);
  
    juce::ComponentDragger dragger;
    BoundsContrainer constrainer{};
    HorizontalConstrainer hConstrainer{};
    
    juce::ListenerList<Listener> nodeListeners;
    
    juce::AudioProcessorValueTreeState& apvts;
};

