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
#ifndef __SCENE_EDITOR_MODULE_H__
#define __SCENE_EDITOR_MODULE_H__

#include "omega/Engine.h"
#include "omega/Application.h"
#include "omega/Actor.h"
#include "omega/SceneNode.h"
#include "omegaToolkitConfig.h"

namespace omegaToolkit
{
	using namespace omega;
	class SceneEditorModule;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	class EditableObject
	{
	public:
		EditableObject(SceneNode* node, SceneEditorModule* editor);

		omega::SceneNode* getSceneNode() { return mySceneNode; }
		SceneEditorModule* getEditor() { return myEditor; }

	private:
		SceneNode* mySceneNode;
		SceneEditorModule* myEditor;
		String myName;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	class OTK_API SceneEditorModule: public EngineModule
	{
	public:
		enum InteractorStyle { MouseInteractorStyle, ControllerInteractorStyle };
		
	public:
		static SceneEditorModule* createAndInitialize();
		virtual ~SceneEditorModule();

		void initialize();
		void update(const UpdateContext& context);
		void handleEvent(const Event& evt);

		void setInteractor(Actor* interactor) { myInteractor = interactor; }
		Actor* getInteractor() { return myInteractor; }

		void addNode(SceneNode* node);
		void removeNode(SceneNode* node);

		SceneNode* getSelectedNode();

		bool isEnabled() { return myEnabled; }
		void setEnabled(bool value) { myEnabled = value; }

	private:
		SceneEditorModule();
		EditableObject* findEditableObject(SceneNode* node);
		void updateSelection(const Ray& ray);

	private:
		bool myEnabled;
		Actor* myInteractor;

		EditableObject* mySelectedObject;
		List<EditableObject*> myObjects;
	};
};
#endif