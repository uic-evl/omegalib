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
#include <osgUtil/Optimizer>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Material>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaOsg.h>

// begin of #include from osgBullet example
#include <btBulletCollisionCommon.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgwTools/Shapes.h>
#include <osgwTools/Version.h>
#include <osg/ShapeDrawable>

#include <osg/io_utils>
#include <iostream>
// end of #include from osgBullet exaple

using namespace omega;
using namespace omegaToolkit;
using namespace omegaOsg;

String sModelName;
float sModelSize = 1.0f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Collision: public EngineModule
{
public:
	Collision()
	{
		myOsg = new OsgModule();
		ModuleServices::addModule(myOsg);
		myColWorld = initCollision();
	}

	virtual btCollisionWorld* initCollision();

	virtual void initialize();
	virtual void update(const UpdateContext& context);
	//virtual void handleEvent(const Event& evt) {}

private:
	btCollisionWorld* myColWorld;
	Ref<OsgModule> myOsg;
	Ref<SceneNode> mySceneNode;
	//osgViewer::Viewer myViewer; // osgb viewer
	//MoveManipulator * myMoveManipulator; // osgb manipulator
	Actor* myInteractor; // omegaLib interactor
	//osg::ref_ptr<osg::Light> myLight;
	bool myLastColState;
	btCollisionObject* myColObject;
	OsgSceneObject* myOso;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
btCollisionWorld* Collision::initCollision()
{
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher( collisionConfiguration );

    btVector3 worldAabbMin( -100, -100, -100 );
    btVector3 worldAabbMax( 100, 100, 100 );
    btBroadphaseInterface* inter = new btAxisSweep3( worldAabbMin, worldAabbMax, 1000 );

    btCollisionWorld* collisionWorld = new btCollisionWorld( dispatcher, inter, collisionConfiguration );

    return( collisionWorld );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Collision::initialize()
{
	// The node containing the scene
	osg::ref_ptr< osg::Group > root = new osg::Group();

	//root->addChild( createAxes().get() );

	// create 1st box, static
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable( osgwTools::makeBox( osg::Vec3( 1, 1, 1) ) );
	root->addChild( geode.get() );

	// make it a bullet object
	myColObject = new btCollisionObject();
	myColObject->setCollisionShape( osgbCollision::btBoxCollisionShapeFromOSG( geode ) ); // <-- here a Bullet object's shape is defined as a leaf node of OSG
    myColObject->setCollisionFlags( btCollisionObject::CF_STATIC_OBJECT );
	myColObject->setWorldTransform( osgbCollision::asBtTransform( osg::Matrix::translate(0,0,0) ) );
    myColWorld->addCollisionObject( myColObject );

	// create 2nd box, draggable
	geode = new osg::Geode;
    geode->addDrawable( osgwTools::makeBox( osg::Vec3( 1.5, 1.5, 1.5 ) ) );
    //osg::Matrix transMatrix = osg::Matrix::translate( 3., 0., 0. );
	//osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform( transMatrix );
	//mt->addChild( geode.get() );

	// make it a bullet object
	myColObject = new btCollisionObject();
    myColObject->setCollisionShape( osgbCollision::btBoxCollisionShapeFromOSG( geode ) );
    myColObject->setCollisionFlags( btCollisionObject::CF_KINEMATIC_OBJECT );
    //myColObject->setWorldTransform( osgbCollision::asBtTransform( transMatrix ) );
    myColWorld->addCollisionObject( myColObject );
	// following two lines are why this object is draggable
    //myMoveManipulator->setCollisionObject( btBoxObject );
    //myMoveManipulator->setMatrixTransform( mt );
	//*/ //end //

	//Optimize scenegraph
	//osgUtil::Optimizer optOSGFile;
	//optOSGFile.optimize(root);

	// Create an omegalib scene node and attach the osg node to it. This is used to interact with the 
	// osg object through omegalib interactors.
	myOso = new OsgSceneObject(geode.get());
	root->addChild( myOso->getTransformedNode() );
	mySceneNode = new SceneNode(getEngine());
	mySceneNode->addComponent(myOso);
	mySceneNode->setBoundingBoxVisible(true);
	mySceneNode->setPosition(3,0,0);
	//mySceneNode->setBoundingBoxVisible(false);
	getEngine()->getScene()->addChild(mySceneNode);
	//getEngine()->getDefaultCamera()->focusOn(getEngine()->getScene());
	getEngine()->getDefaultCamera()->setPosition(0,-2,30);

    // Set the interactor style used to manipulate meshes.
	if(SystemManager::settingExists("config/interactor"))
	{
		Setting& sinteractor = SystemManager::settingLookup("config/interactor");
		myInteractor = ToolkitUtils::createInteractor(sinteractor);
		if(myInteractor != NULL)
		{
			ModuleServices::addModule(myInteractor);
		}
	}

	if(myInteractor != NULL)
	{
		myInteractor->setSceneNode(mySceneNode);
	}
	//*/

	// Set the osg node as the root node
	myOsg->setRootNode(root);
	//*/

	/*/ create viewer (using osg funcdtions to view the scene and handle event, as in osgb)
    myViewer.setUpViewInWindow( 10, 30, 800, 600 );
    myViewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    myViewer.addEventHandler( myMoveManipulator );
    myViewer.setSceneData( root.get() );
	//*/
	
	/*/ Setup shading
	myLight = new osg::Light;
    myLight->setLightNum(0);
    myLight->setPosition(osg::Vec4(0.0, 2, 1, 1.0));
    myLight->setAmbient(osg::Vec4(0.1f,0.1f,0.2f,1.0f));
    myLight->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	myLight->setSpotExponent(0);
	myLight->setSpecular(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
	//*/
	//osg::LightSource* ls = new osg::LightSource();
	//ls->setLight(myLight);
	//ls->setLocalStateSetModes(osg::StateAttribute::ON);
	//ls->setStateSetModes(*root->getOrCreateStateSet(), osg::StateAttribute::ON);

	//root->addChild(ls);

	myLastColState = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectCollision( bool& lastColState, btCollisionWorld* cw )
{
    unsigned int numManifolds = cw->getDispatcher()->getNumManifolds();
    if( ( numManifolds == 0 ) && (lastColState == true ) )
    {
        osg::notify( osg::ALWAYS ) << "No collision." << std::endl;
        lastColState = false;
    }
    else {
        for( unsigned int i = 0; i < numManifolds; i++ )
        {
            btPersistentManifold* contactManifold = cw->getDispatcher()->getManifoldByIndexInternal(i);
            unsigned int numContacts = contactManifold->getNumContacts();
            for( unsigned int j=0; j<numContacts; j++ )
            {
                btManifoldPoint& pt = contactManifold->getContactPoint( j );
                if( ( pt.getDistance() <= 0.f ) && ( lastColState == false ) )
                {
                    // grab these values for the contact normal arrows:
                    osg::Vec3 pos = osgbCollision::asOsgVec3( pt.getPositionWorldOnA() ); // position of the collision on object A
                    osg::Vec3 normal = osgbCollision::asOsgVec3( pt.m_normalWorldOnB ); // returns a unit vector
                    float pen = pt.getDistance(); //penetration depth

                    osg::Quat q;
                    q.makeRotate( osg::Vec3( 0, 0, 1 ), normal );

                    osg::notify( osg::ALWAYS ) << "Collision detected." << std::endl;

                    osg::notify( osg::ALWAYS ) << "\tPosition: " << pos << std::endl;
                    osg::notify( osg::ALWAYS ) << "\tNormal: " << normal << std::endl;
                    osg::notify( osg::ALWAYS ) << "\tPenetration depth: " << pen << std::endl;
                    //osg::notify( osg::ALWAYS ) << q.w() <<","<< q.x() <<","<< q.y() <<","<< q.z() << std::endl;
                    lastColState = true;
                }
                else if( ( pt.getDistance() > 0.f ) && ( lastColState == true ) )
                {
                    osg::notify( osg::ALWAYS ) << "No collision." << std::endl;
                    lastColState = false;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Collision::update(const UpdateContext& context)
{
	osg::Matrix m = myOso->getTransformedNode()->getMatrix();
	//std::printf("(%lf, %lf, %lf)\n", m.getTrans().x(),m.getTrans().y(),m.getTrans().z());
	myColObject->setWorldTransform( osgbCollision::asBtTransform( m ) );
	myColWorld->performDiscreteCollisionDetection();
    detectCollision( myLastColState, myColWorld );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Application entry point
int main(int argc, char** argv)
{
	Application<Collision> app("osgBulletTest");
    return omain(app, argc, argv);
}