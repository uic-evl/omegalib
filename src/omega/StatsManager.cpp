/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory,  
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
#include "omega/StatsManager.h"
#include "omega/DrawInterface.h"
#include "omega/SystemManager.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
Stat* Stat::create(const String& name, StatsManager::StatType type)
{
	StatsManager* sm = SystemManager::instance()->getStatsManager();
	return sm->createStat(name, type);
}

///////////////////////////////////////////////////////////////////////////////
Stat* Stat::find(const String& name)
{
	StatsManager* sm = SystemManager::instance()->getStatsManager();
	return sm->findStat(name);
}

///////////////////////////////////////////////////////////////////////////////
StatsManager::StatsManager():
myStatMask(0)
{
}

///////////////////////////////////////////////////////////////////////////////
Stat* StatsManager::createStat(const String& name, StatType type)
{
	if(findStat(name) == NULL)
	{
		Stat* s = new Stat(this, name, type);
		myStatDictionary[name] = s;
		myStatList.push_back(s);
	}
	else
	{
		ofwarn("StatsManager::createStat: stat %1% already exists", %name);
	}
	return findStat(name);
}

///////////////////////////////////////////////////////////////////////////////
void StatsManager::removeStat(Stat* s)
{
	oassert(s != NULL);
	myStatList.remove(s);
}

///////////////////////////////////////////////////////////////////////////////
Stat* StatsManager::findStat(const String& name)
{
	return myStatDictionary[name];
}

///////////////////////////////////////////////////////////////////////////////
List<Stat*>::Range StatsManager::getStats()
{
	return List<Stat*>::Range(myStatList.begin(), myStatList.end());
}

///////////////////////////////////////////////////////////////////////////////
void StatsManager::printStats()
{
	omsg("-------------------------------------------------------------------------------- STATS");
	omsg("NAME        CUR      MIN      MAX      AVG");
	foreach(Stat* s, myStatList)
	{
	    if(s->isValid())
		{
		ofmsg("%-11s %-8.1f %-8.1f %-8.1f %-8.1f", %s->getName().c_str() %s->getCur() %s->getMin() %s->getMax() %s->getAvg());
		}
	}
	omsg("-------------------------------------------------------------------------------- STATS");
}
