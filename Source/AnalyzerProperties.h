/*
  ==============================================================================

    AnalyzerProperties.h
    Created: 17 May 2022 9:33:59pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace AnalyzerProperties
{


enum class ParamNames
{
    EnableAnalyzer,
    AnalyzerDecayRate,
    AnalyzerPoints,
    AnalyzerProcessingMode
};

enum class FFTOrder
{
    FFT2048 = 11,
    FFT4096 = 12,
    FFT8192 = 13
};

enum class ProcessingModes
{
    Pre,
    Post
};

inline const std::map<ParamNames, juce::String>& GetAnalyzerParams()
{
    static const std::map<ParamNames, juce::String> map =
    {
        {ParamNames::EnableAnalyzer, "Enable Analyzer"},
        {ParamNames::AnalyzerDecayRate, "Analyzer Decay Rate"},
        {ParamNames::AnalyzerPoints, "Analyzer Points"},
        {ParamNames::AnalyzerProcessingMode, "Analyzer Proc Mode"}
    };
    
    return map;
}

inline const std::map<FFTOrder, juce::String>& GetAnalyzerPoints()
{
    static const std::map<FFTOrder, juce::String> map =
    {
        {FFTOrder::FFT2048, "2048"},
        {FFTOrder::FFT4096, "4096"},
        {FFTOrder::FFT8192, "8192"}
    };
    
    return map;
}

inline const std::map<ProcessingModes, juce::String>& GetProcessingModes()
{
    static const std::map<ProcessingModes, juce::String> map =
    {
        {ProcessingModes::Pre, "Pre"},
        {ProcessingModes::Post, "Post"}
    };

    return map;
}


inline const juce::String getAnalyzerParamName(ParamNames name)
{
    auto const params = GetAnalyzerParams();
    return params.at(name);
}


inline void  addAnalyzerParams(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    const auto& params = GetAnalyzerParams();
    
   
    layout.add(std::make_unique<juce::AudioParameterBool>(params.at(ParamNames::EnableAnalyzer),
                                                          "Enable",
                                                          true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(params.at(ParamNames::AnalyzerDecayRate),
                                                          "Decay Rate",
                                                           juce::NormalisableRange<float>(0.0f, 30.0f, 1.0f, 1.0f), 30.0f));
    
    juce::StringArray orders;
    
    for (const auto& [order, stringRep] : GetAnalyzerPoints())
    {
        orders.add(stringRep);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(ParamNames::AnalyzerPoints),
                                                            "FFT Points",
                                                            orders, 1));
      
    juce::StringArray modes;
    
    for (const auto& [mode, stringRep] : GetProcessingModes())
    {
        modes.add(stringRep);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(ParamNames::AnalyzerProcessingMode),
                                                            "Mode",
                                                            modes, 0));
    
    
}


}
