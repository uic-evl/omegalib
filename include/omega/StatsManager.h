/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	Utility classes for tracking various statistics and profiling information
 *  in omegalib apps.
 ******************************************************************************/
#ifndef __STATS_MANAGER__
#define __STATS_MANAGER__

#include "omega/osystem.h"

namespace omega
{
    class DrawInterface;
    class Stat;

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API StatsManager: public ReferenceType
    {
    public:
        enum StatType { Time, Memory, Primitive, Fps, Count1, Count2, Count3, Count4 };
    public:
        StatsManager();

        Stat* createStat(const String& name, StatType type);
        Stat* findStat(const String& name);
        void removeStat(Stat* s);
        List<Stat*>::Range getStats();
        void printStats();

    private:
        Dictionary<String, Stat*> myStatDictionary;
        // List of stats. Stats are normal pointers, since we want to leave
        // stat ownership to user code. When a stat reference count goes to
        // zero, the stat will remove itself from this list.
        List< Stat* > myStatList;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API Stat: public ReferenceType
    {
    friend class StatsManager;
    public:
        //! Creation method, for consistency with python API
        static Stat* create(const String& name, StatsManager::StatType type);
        //! Additional 'creation' method used to find an existing Stat object.
        //! Put in the Stat class for ease-of-use from python scripts.
        static Stat* find(const String& name);

        Stat(StatsManager* owner):
          myOwner(owner) {}

        virtual ~Stat();

        const String& getName();
        bool isValid();
        
        //! Starts timing this statistic. Valid only for Time type stats.
        void startTiming();
        //! Stops timing this statistic and adds a sample of the elapsed time
        //! (since startTiming was called) in milliseconds. Valid only for 
        //! Time type stats.
        void stopTiming();
        void addSample(double sample);

        StatsManager::StatType getType() { return myType; }
        
        int getNumSamples();
        float getCur();
        float getMin();
        float getMax();
        float getAvg();
        float getTotal();

    private:
        Stat(StatsManager* owner, const String& name, StatsManager::StatType type): 
           myName(name), myValid(false), myNumSamples(0), myType(type), myOwner(owner) {}

    private:
        Ref<StatsManager> myOwner;
        bool myValid;
        String myName;
        double myCur;
        double myMin;
        double myMax;
        double myAvg;
        int myNumSamples;
        double myAccumulator;
        StatsManager::StatType myType;

        Timer myTimer;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline void Stat::startTiming()
    {
        if(myType == StatsManager::Time)
        {
            myTimer.start();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    inline Stat::~Stat()
    {
        myOwner->removeStat(this);
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void Stat::stopTiming()
    {
        if(myType == StatsManager::Time)
        {
            myTimer.stop();
            addSample(myTimer.getElapsedTimeInMilliSec());
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void Stat::addSample(double sample)
    {
        if(!myValid)
        {
            // First sample. Initialize the statistics
            myAccumulator = sample;
            myCur = sample;
            myMin = sample;
            myMax = sample;
            myAvg = sample;
            myNumSamples = 1;
            myValid = true;
        }
        else
        {
            // Update the statistics.
            myAccumulator += sample;
            myCur = sample;
            myNumSamples++;
            if(sample < myMin) myMin = sample;
            if(sample > myMax) myMax = sample;
            myAvg = myAccumulator / myNumSamples;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    inline const String& Stat::getName()
    { return myName; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Stat::isValid()
    { return myValid; }

    ///////////////////////////////////////////////////////////////////////////
    inline int Stat::getNumSamples()
    { return myNumSamples; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Stat::getCur()
    { oassert(myValid); return myCur; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Stat::getMin()
    { oassert(myValid); return myMin; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Stat::getMax()
    { oassert(myValid); return myMax; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Stat::getAvg()
    { oassert(myValid); return myAvg; }

    ///////////////////////////////////////////////////////////////////////////
    inline float Stat::getTotal()
    { oassert(myValid); return myAccumulator;	}
}; // namespace omega

#endif