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

}
