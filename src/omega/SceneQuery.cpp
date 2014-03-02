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
#include "omega/SceneQuery.h"

using namespace omega;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneQuery::clearResults()
{
	myResults.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const SceneQueryResultList& RaySceneQuery::execute(uint flags)
{
	bool queryOne = ((flags & SceneQuery::QueryFirst) == SceneQuery::QueryFirst) ? true : false;

	queryNode(myScene, myResults, queryOne);

	if(((flags & SceneQuery::QuerySort) == SceneQuery::QuerySort) && myResults.size() > 1)
	{
		myResults.sort(SceneQueryResultDistanceCompare);
	}
	return myResults;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RaySceneQuery::queryNode(SceneNode* node, SceneQueryResultList& list, bool queryFirst)
{
	bool selected = false;
	if(node->isSelectable() && node->isVisible())
	{
		Vector3f hitPoint;
		if(node->hit(myRay, &hitPoint, SceneNode::HitBest))
		{
			SceneQueryResult res;
			res.node = node;
			res.hitPoint = hitPoint;
			res.distance = (hitPoint - myRay.getOrigin()).norm();
			list.push_back(res);
		}
	}

	// If this node has been selected, and we are returning the first node found, return immediately.
	if(selected && queryFirst) return;

	// Draw children nodes.
	for(int i = 0; i < node->numChildren(); i++)
	{
		SceneNode* n = (SceneNode*)node->getChild(i);
		queryNode(n, list, queryFirst);
	}
}

