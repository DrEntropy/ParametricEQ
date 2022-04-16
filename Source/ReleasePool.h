/*
  ==============================================================================

    ReleasePool.h
    Created: 16 Apr 2022 9:50:11am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

template <typename ObjectType>
struct ReleasePool : Timer {
    using Ptr = ReferenceCountedObjectPtr<ObjType>
    using Array = RefferenceCountedArray<ObjType>
    ReleasePool():Timer{}
    {
        // nothing
    }
    
    void add(Ptr ptr)
    {
        // add
    }
    
    void add (Array array)
    {
        //add the individual elements
    }
    
    void timerCallback() override
    {
        
    }
    
private:
    
    void addIfNotAlreadyThere(Ptr ptr)
    {
    
    }
};
