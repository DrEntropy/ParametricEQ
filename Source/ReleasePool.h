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
 
    ReleasePool(size_t delPoolSize, int interval) : Timer{}
    {
        deletionPool.reserve(delPoolSize);
        startTimer(interval); // two seconds
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
            jassert(newAddition.value);
        }
    }
    
 
    
    void timerCallback() override
    {
       if (newAddition.compareAndSetBool (false, true))
       {
           Ptr object;
           while (holdFifo.getNumAvailableForReading() >0)
           {
               bool exchangeSucceeded = holdFifo.exchange(std::move(object));
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
        if(std::find(deletionPool.begin(), deletionPool.end(), ptr) == deletionPool.end())
            deletionPool.push_back(ptr);
    }
    
    static bool readyToDelete(const Ptr& ptr)
    {
        return ptr.get()->getReferenceCount() <= 1;
    }
    
    std::vector<Ptr> deletionPool;
    Fifo<Ptr, PoolSize> holdFifo;
    juce::Atomic<bool> newAddition {false};

    // for debugging
    //int count{0};
    
};
