/*
  ==============================================================================

    ReleasePool.h
    Created: 16 Apr 2022 9:50:11am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Fifo.h"

template <typename ObjectType, size_t PoolSize>
struct ReleasePool : juce::Timer {
    using Ptr = ReferenceCountedObjectPtr<ObjType>
    using Array = RefferenceCountedArray<ObjType>
    
    ReleasePool(size_t delPoolSize) : Timer{}
    {
        deletionPool.reserve(delPoolSize);
        startTimer(2000); // two seconds
    }
    
    void add(Ptr ptr)
    {
        auto messManager = juce::MessageManager::getInstanceWithoutCreating();
        if (messManager && messManger.isThisMessageThread())
        {
            // message thread, add to pool directly
            addIfNotAlreadyThere(ptr);
        }
        else
        {
            // on audio thread, push to fifo to deal with later.
            newAddition = holdPool.push(ptr);
            jassert(newAddition);
        }
    }
    
    void add (Array array)
    {
        //TODO loop through array andd add each element
        
    }
    
    void timerCallback() override
    {
       if (newAddition.compareAndSetBool (false, true))
       {
           Ptr object;
           while (holdPool.getNumAvailableForReading() >0)
           {
               paramFifo.exchange(object);
               addIfNotAlreadyThere(object);
               object = nullptr; // ready for next one!
           }
       }
       // Ok time to clean the pool.
        deletionPool.erase(std::remove_if(deletionPool.begin(), deletionPool.end(), readyToDelete), deletionPool.end());
        
    }
    
private:
    
    void addIfNotAlreadyThere(Ptr ptr)
    {
      //TODO use std::find to check if ptr is in the pool. if it is not, then pushback
    }
    
    bool readyToDelete(const Ptr& ptr)
    {
        return ptr.get()->getReferenceCount() <= 1;
    }
    
    std::vector<Ptr> deletionPool;
    Fifo<Ptr, PoolSize> holdPool;
    Atomic<bool> newAddition {false};
    
};
