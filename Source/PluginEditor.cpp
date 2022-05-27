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
    : AudioProcessorEditor (&p), audioProcessor (p), analyzerControls(p.apvts), responseCurve(p.getSampleRate(), p.apvts)

{
    spectrumAnalyzer.reset(new SpectrumAnalyzer<juce::AudioBuffer<float>> (audioProcessor.getSampleRate(), audioProcessor.leftSCSFifo, audioProcessor.rightSCSFifo, audioProcessor.apvts));
    
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(eqParamContainer);
    
    addAndMakeVisible(analyzerControls);
    
    addAndMakeVisible(bypassButtonContainer);
    
    addAndMakeVisible(globalBypass);
    addAndMakeVisible(*spectrumAnalyzer);
    addAndMakeVisible(responseCurve);
 
    setSize (1200, 800);
    

    audioProcessor.addSampleRateListener(this);
    
    
    startTimerHz(FRAME_RATE);
    
    audioProcessor.editorActive = true;
    
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
    audioProcessor.removeSampleRateListener(this);
    audioProcessor.editorActive = false;
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
    responseCurve.setBounds(centerBounds.reduced(PARAM_CONTROLS_MARGIN));
    
    // for future use, make room for square bounded controls
    auto controlWidth = bottomBounds.getHeight();
    
    //controls are square:
    auto inTrimBounds = bottomBounds.removeFromLeft(controlWidth);
    auto outTrimBounds = bottomBounds.removeFromRight(controlWidth);
    bottomBounds.removeFromLeft(controlWidth / 2); //space between in trim and proc mode
    auto procModeBounds = bottomBounds.removeFromLeft(controlWidth);
    
    // analyzer control has 4 buttons, but make it 4.5 = 9/2 for abit extra room.
    auto analyzerControlBounds = bottomBounds.removeFromLeft(bottomBounds.getHeight() * 9  / 2);
    
    analyzerControls.setBounds(analyzerControlBounds);
    
    auto resetAllBounds = bottomBounds; //Placeholder
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
    
#if USE_TEST_OSC
    int step_time = JUCE_LIVE_CONSTANT(120);
    if(counter >= step_time)
    {
        auto sampleRate = audioProcessor.getSampleRate();
        
        using namespace AnalyzerProperties;
        auto fftOrder = getFFTOrder(audioProcessor.apvts.getParameter(getAnalyzerParamName(ParamNames::AnalyzerPoints))->getValue());
        auto fftSize = 1 << static_cast<int>(fftOrder);
        auto numBins = fftSize / 2 + 1;
        decltype(numBins) maxBin = std::ceil(20000.f * fftSize / sampleRate); //largest bin we care about.
        
        auto bin = audioProcessor.binNum.load();
        if(++bin >= std::min(numBins, maxBin))
            bin = 0;
        
        DBG(bin * sampleRate / fftSize);
        
        audioProcessor.binNum.store(bin);
        counter = 1;
    }
    else if (JUCE_LIVE_CONSTANT(false))
    {
        ++counter;
    }
#endif

}

// functionality to be moved into SpectrumAnalyzer class
void ParametricEQAudioProcessorEditor::sampleRateChanged(double sr)
{
    if(spectrumAnalyzer)
        spectrumAnalyzer->changeSampleRate(sr);
}
