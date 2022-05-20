/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MeterValues.h"

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //TODO, deal with case where sample rate changes. Editor is not going to get reconstructed! but this is just a placeholder
    spectrumAnalyzer.reset(new SpectrumAnalyzer<juce::AudioBuffer<float>> (audioProcessor.getSampleRate(), audioProcessor.leftSCSFifo, audioProcessor.rightSCSFifo, audioProcessor.apvts));
    
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(eqParamContainer);
    
    addAndMakeVisible(bypassButtonContainer);
    
    addAndMakeVisible(globalBypass);
    addAndMakeVisible(*spectrumAnalyzer);
 
    setSize (1200, 800);
    

    audioProcessor.addSampleRateListener(this);
    
    
    startTimerHz(FRAME_RATE);
    
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
    audioProcessor.removeSampleRateListener(this);
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromFloatRGBA (0.1f, 0.1f, 0.2f, 1.0f));
    

}

void ParametricEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    auto bottomBounds = bounds.removeFromBottom(BOTTOM_CONTROLS_HEIGHT); // placeholder for bottom controls
 
    bounds.reduce(OVERALL_MARGIN, OVERALL_MARGIN);
    
    inputMeter.setBounds(bounds.removeFromLeft(SCALE_AND_METER_WIDTH));
    outputMeter.setBounds(bounds.removeFromRight(SCALE_AND_METER_WIDTH));
    
    //  These magic numbers are placeholders
    eqParamContainer.setBounds(bounds.removeFromBottom(PARAM_CONTROLS_HEIGHT).reduced(PARAM_CONTROLS_MARGIN));
    
    auto topBounds = bounds.removeFromTop(BYPASS_SWITCH_HEIGHT + 2 * BYPASS_SWITCH_V_MARGIN);

    bypassButtonContainer.setBounds(bounds.removeFromTop(BYPASS_SWITCH_HEIGHT));
    
    
    globalBypass.setBounds(topBounds.withTrimmedBottom(2 * BYPASS_SWITCH_V_MARGIN)
                                    .withTrimmedRight(GLOBAL_SWITCH_RIGHT_MARGIN).removeFromRight(2 * BYPASS_SWITCH_HEIGHT));
    
    auto centerBounds = bounds;
    spectrumAnalyzer->setBounds(centerBounds.reduced(PARAM_CONTROLS_MARGIN));
}


void ParametricEQAudioProcessorEditor::timerCallback()
{
    auto& inputFifo = audioProcessor.inMeterValuesFifo;
    auto& outputFifo = audioProcessor.outMeterValuesFifo;
    
    MeterValues values;
    
    
    if(inputFifo.getNumAvailableForReading() > 0)
    {
        while(inputFifo.pull(values))
        {
            // nothing  
        }
        
        inputMeter.update(values);
    }
    
    if(outputFifo.getNumAvailableForReading() > 0)
    {
        while(outputFifo.pull(values))
        {
            // nothing
        }
        
        outputMeter.update(values);
    }
    

}

// functionality to be moved into SpectrumAnalyzer class
void ParametricEQAudioProcessorEditor::sampleRateChanged(double sr)
{
    if(spectrumAnalyzer)
        spectrumAnalyzer->changeSampleRate(sr);
}
