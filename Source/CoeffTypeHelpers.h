/*
  ==============================================================================

    CoeffTypeHelpers.h
    Created: 19 Apr 2022 1:39:33pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once


// helpers for type checking
template <typename U>
struct isReferenceCountedObjectPtr : std::false_type { };

template <typename W>
struct isReferenceCountedObjectPtr<juce::ReferenceCountedObjectPtr<W>> : std::true_type { };

template <typename U>
struct isReferenceCountedArray : std::false_type { };

template <typename W>
struct isReferenceCountedArray<juce::ReferenceCountedArray<W>> : std::true_type { };

template <typename U>
struct isVector : std::false_type { };

template <typename W>
struct isVector<std::vector<W>> : std::true_type { };

template <typename U>
struct isAudioBuffer : std::false_type { };

template <typename W>
struct isAudioBuffer<juce::AudioBuffer<W>> : std::true_type { };
