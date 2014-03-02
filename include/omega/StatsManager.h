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
	///////////////////////////////////////////////////////////////////////////
	class OMEGA_API Stat
	{
	friend class StatsManager;
	public:
		enum Type { Time, Memory, Primitive, Fps, Count1, Count2, Count3, Count4 };
	public:
		const String& getName();
		bool isValid();
		
		//! Starts timing this statistic. Valid only for Time type stats.
		void startTiming();
		//! Stops timing this statistic and adds a sample of the elapsed time
		//! (since startTiming was called) in milliseconds. Valid only for 
		//! Time type stats.
		void stopTiming();
		void addSample(double sample);

		Type getType() { return myType; }
		
		int getNumSamples();
		float getCur();
		float getMin();
		float getMax();
		float getAvg();
		float getTotal();

	private:
		Stat(const String& name, Type type): 
		   myName(name), myValid(false), myNumSamples(0), myType(type) {}

	private:
		bool myValid;
		String myName;
		double myCur;
		double myMin;
		double myMax;
		double myAvg;
		int myNumSamples;
		double myAccumulator;
		Type myType;

		Timer myTimer;
	};

	///////////////////////////////////////////////////////////////////////////
	class OMEGA_API StatsManager
	{
	public:
		StatsManager();

		Stat* createStat(const String& name, Stat::Type type);
		Stat* findStat(const String& name);
		List<Stat*>::Range getStats();
		void printStats();

	private:
		Dictionary<String, Stat*> myStatDictionary;
		List<Stat*> myStatList;
	};

	///////////////////////////////////////////////////////////////////////////
	inline void Stat::startTiming()
	{
		if(myType == Time)
		{
			myTimer.start();
		}
	}

	///////////////////////////////////////////////////////////////////////////
	inline void Stat::stopTiming()
	{
		if(myType == Time)
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