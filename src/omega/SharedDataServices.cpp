/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/
#include "omega/SharedDataServices.h"
#include "eqinternal/eqinternal.h"

using namespace omega;

SharedData* SharedDataServices::mysSharedData = NULL;
Dictionary<String, SharedObject*> SharedDataServices::mysRegistrationQueue;

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedOStream::write( const void* data, uint64_t size )
{ 
	myStream->write(data, size); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SharedOStream& SharedOStream::operator<< ( const String& str )
{ 
	const uint64_t nElems = str.length();
	write( &nElems, sizeof( nElems ));
	if ( nElems > 0 )
		write( str.c_str(), nElems );

		
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedIStream::read( void* data, uint64_t size )
{ 
	myStream->read(data, size); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SharedIStream& SharedIStream::operator>> ( String& str )
{ 
	uint64_t nElems = 0;
	read( &nElems, sizeof( nElems ));
	if(nElems > myStream->getRemainingBufferSize())
	{
	   oferror("SHaredDataServices: nElems(%1%) > getRemainingBufferSize(%2%)",
	   %nElems %myStream->getRemainingBufferSize());
	}
	oassert( nElems <= myStream->getRemainingBufferSize());
	if( nElems == 0 )
		str.clear();
	else
	{
		str.assign( static_cast< const char* >( myStream->getRemainingBuffer( )), 
					nElems );
		myStream->advanceBuffer( nElems );
	}
	return *this; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedData::registerObject(SharedObject* module, const String& sharedId)
{
	//ofmsg("SharedData::registerObject: registering %1%", %sharedId);
	myObjects[sharedId] = module;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedData::unregisterObject(const String& sharedId)
{
	//ofmsg("SharedData::unregisterObject: unregistering %1%", %sharedId);
	myObjects.erase(sharedId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedData::getInstanceData( co::DataOStream& os )
{
	//omsg("#### SharedData::getInstanceData");
	SharedOStream out(&os);

	// Serialize update context.
	out << myUpdateContext.frameNum << myUpdateContext.dt << myUpdateContext.time;

	int numObjects = myObjects.size();
	out << numObjects;

	foreach(SharedObjectItem obj, myObjects)
	{
		out << obj.getKey();
		obj->commitSharedData(out);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedData::applyInstanceData( co::DataIStream& is )
{
	//omsg("#### SharedData::applyInstanceData");
	SharedIStream in(&is);

	// Desrialize update context.
	in >> myUpdateContext.frameNum >> myUpdateContext.dt >> myUpdateContext.time;

	int numObjects;
	in >> numObjects;

	while(numObjects > 0)
	{
		String objId;
		in >> objId;

		SharedObject* obj = myObjects[objId];
		if(obj != NULL)
		{
			obj->updateSharedData(in);
		}
		else
		{
			oferror("FATAL ERROR: SharedDataServices::applyInstanceData: could not find object key %1%", %objId);
		}

		numObjects--;
	};
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedDataServices::setSharedData(SharedData* data)
{
	mysSharedData = data;
	typedef Dictionary<String, SharedObject*>::Item SharedObjectEntry;
	foreach(SharedObjectEntry item, mysRegistrationQueue)
	{
		registerObject(item.getValue(), item.getKey());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedDataServices::registerObject(SharedObject* module, const String& sharedId)
{
	if(mysSharedData != NULL) 
	{
		mysSharedData->registerObject(module, sharedId);
	}
	else
	{
		ofmsg("SharedDataServices::registerObject: queuing %1% for registration", %sharedId);
		// QUEUE
		mysRegistrationQueue[sharedId] = module;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedDataServices::unregisterObject(const String& id)
{
	if(mysSharedData != NULL) 
	{
		mysSharedData->unregisterObject(id);
	}
	else
	{
		oferror("SharedDataServices::unregisterObject: shared data stream unavailable while unregistering %1%", %id);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SharedDataServices::cleanup()
{
	// Shared data should take care of cleanup internally, here we just clean up the queue.
	mysRegistrationQueue.clear();
}
