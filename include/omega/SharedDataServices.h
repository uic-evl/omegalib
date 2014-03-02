/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#ifndef __SHARED_DATA_SERVICES__
#define __SHARED_DATA_SERVICES__

#include "omega/osystem.h"

namespace co
{
    class DataOStream;
    class DataIStream;
};

namespace omega
{
	class SharedData;
	class EngineModule;

	///////////////////////////////////////////////////////////////////////////////////////////////
    class OMEGA_API SharedOStream
    {
    public:
		SharedOStream(co::DataOStream* stream): myStream(stream) {}

        template< typename T > SharedOStream& operator << ( const T& value )
        { write( &value, sizeof( value )); return *this; }

		SharedOStream& operator << ( const String& str );
	
		void write( const void* data, uint64_t size );

		co::DataOStream* getInternalStream() { return myStream; }
	
	private:
		co::DataOStream* myStream;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
    class OMEGA_API SharedIStream
    {
    public:
		SharedIStream(co::DataIStream* stream): myStream(stream) {}

        template< typename T >
        SharedIStream& operator >> ( T& value )
            { read( &value, sizeof( value )); return *this; }

		SharedIStream& operator >> ( String& str );
	
		void read( void* data, uint64_t size );
	
		co::DataIStream* getInternalStream() { return myStream; }

	private:
		co::DataIStream* myStream;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API SharedObject: public ReferenceType
	{
	public:
		virtual void commitSharedData(SharedOStream& out) {}
		virtual void updateSharedData(SharedIStream& in) {}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API SharedDataServices
	{
	public:
		static void setSharedData(SharedData* data);
		static void registerObject(SharedObject*, const String& id);
		static void unregisterObject(const String& id);
		static void cleanup();

	private:
		static SharedData* mysSharedData;
		static Dictionary<String, SharedObject*> mysRegistrationQueue;
	};
}; // namespace omega

#endif