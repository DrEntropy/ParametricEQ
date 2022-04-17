/*
  ==============================================================================

    ReleasePool.h
    Created: 16 Apr 2022 9:50:11am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include <algorithm>
#include <JuceHeader.h>
#include "Fifo.h"

template <typename ObjectType, size_t PoolSize>
struct ReleasePool : juce::Timer {
    
    using Ptr = juce::ReferenceCountedObjectPtr<ObjectType>;
    using Array = juce::ReferenceCountedArray<ObjectType>;
    
    ReleasePool(size_t delPoolSize) : Timer{}
    {
        deletionPool.reserve(delPoolSize);
        startTimer(2000); // two seconds
    }
    
    void add(Ptr ptr)
    {
        auto messManager = juce::MessageManager::getInstanceWithoutCreating();
        if (messManager && messManager->isThisTheMessageThread())
        {
            // message thread, add to pool directly
            addIfNotAlreadyThere(ptr);
        }
        else
        {
            // on audio thread, push to fifo to deal with later.
            newAddition = holdFifo.push(ptr);
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
           while (holdFifo.getNumAvailableForReading() >0)
           {
               bool exchangeSucceeded = holdFifo.exchange(object);
               jassert(exchangeSucceeded); // should not fail.
               if(object.get())
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
        if(std::find(deletionPool.begin(), deletionPool.end(), ptr))
           return;
        
        deletionPool.push_back(ptr);
    }
    
    static bool readyToDelete(const Ptr& ptr)
    {
        return ptr.get()->getReferenceCount() <= 1;
    }
    
    std::vector<Ptr> deletionPool;
    Fifo<Ptr, PoolSize> holdFifo;
    juce::Atomic<bool> newAddition {false};
    
};
