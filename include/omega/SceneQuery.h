/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2013							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __SCENE_QUERY_H__
#define __SCENE_QUERY_H__

#include "osystem.h"
#include "omega/SceneNode.h"

namespace omega {
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Stores a scene query result
	struct SceneQueryResult
	{
		SceneNode* node;
		Vector3f hitPoint;
		float distance;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Comparison function to sort scene query results by distance.
	inline bool SceneQueryResultDistanceCompare(SceneQueryResult& r1, SceneQueryResult& r2)
	{
		if(r1.distance < r2.distance) return false;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// A list of scene query results.
	typedef std::list<SceneQueryResult> SceneQueryResultList;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API SceneQuery
	{
	public:
		enum QueryFlags {QuerySort = 1 << 1, QueryFirst = 1 << 2};

	public:
		SceneQuery(): myScene(NULL) {}

		SceneNode* getSceneNode() { return myScene; }
		void setSceneNode(SceneNode* value) { myScene = value; }

		virtual const SceneQueryResultList& execute(uint flags = 0) = 0;

		void clearResults();

	protected:
		SceneQueryResultList myResults;
		SceneNode* myScene;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API RaySceneQuery: public SceneQuery
	{
	public:
		RaySceneQuery() {}

		void setRay(const Ray& ray) { myRay = ray; }
		const Ray& getRay() { return myRay; }

		virtual const SceneQueryResultList& execute(uint flags = 0);

	private:
		void queryNode(SceneNode* node, SceneQueryResultList& list, bool queryFirst);

	private:
		Ray myRay;
	};
}; // namespace omega

#endif