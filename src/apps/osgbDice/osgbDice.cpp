/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * osgBullet is (C) Copyright 2009-2012 by Kenneth Mark Bryden
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/
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

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgwTools/Shapes.h>
#include <osgwTools/Version.h>
#include <osg/ShapeDrawable>

#include <osgbDynamics/MotionState.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbDynamics/RigidBody.h>
#include <osgbCollision/Utils.h>

#include <btBulletDynamicsCommon.h>


#include <string>

#include <osg/io_utils>
#include <iostream>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaOsg;

osg::MatrixTransform* makeDie( btDynamicsWorld* bw )
{
    osg::MatrixTransform* root = new osg::MatrixTransform;
	const std::string fileName( "dice.osg" );
	String dicePath;
	if(!DataManager::findFile("dice.osg", dicePath))
	{
		osg::notify( osg::FATAL ) << "Can't find \"" << fileName << "\". Make sure OSG_FILE_PATH includes the osgBullet data directory." << std::endl;
		exit( 0 );
	}
    osg::Node* node = osgDB::readNodeFile( dicePath );
    root->addChild( node );

    btCollisionShape* cs = osgbCollision::btBoxCollisionShapeFromOSG( node );
    
    osg::ref_ptr< osgbDynamics::CreationRecord > cr = new osgbDynamics::CreationRecord;
    cr->_sceneGraph = root;
    cr->_shapeType = BOX_SHAPE_PROXYTYPE;
    cr->_mass = 1.f;
    cr->_restitution = 1.f;
    btRigidBody* body = osgbDynamics::createRigidBody( cr.get(), cs );
    bw->addRigidBody( body );

    return( root );
}

// create a box
// 100% osg stuff
osg::Geode* osgBox( const osg::Vec3& center, const osg::Vec3& halfLengths )
{
    osg::Vec3 l( halfLengths * 2. );
    osg::Box* box = new osg::Box( center, l.x(), l.y(), l.z() );
    osg::ShapeDrawable* shape = new osg::ShapeDrawable( box );
    shape->setColor( osg::Vec4( 1., 1., 1., 1. ) );
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable( shape );

    return( geode );
}

//================================================================================

class OsgbDice: public EngineModule
{
public:
	OsgbDice()
	{
		myOsg = new OsgModule();
		ModuleServices::addModule(myOsg);
		myWorld = initBtPhysicsWorld();
	}

	virtual btDynamicsWorld* initBtPhysicsWorld();

	virtual void initialize();
	virtual void update(const UpdateContext& context);
	//virtual void handleEvent(const Event& evt) {}

private:
	btDynamicsWorld* myWorld;
	Ref<OsgModule> myOsg;
	Ref<SceneNode> mySceneNode;
	Actor* myInteractor;
	//osg::ref_ptr<osg::Light> myLight;
	OsgSceneObject* myOso;
	btRigidBody* myShakeBody;
    osg::MatrixTransform* myShakeBox;
	osgbDynamics::MotionState* myShakeMotion;
	osg::MatrixTransform* die1;
	osg::MatrixTransform* die2;
};


// initial steps to create bullet dynamics world
// 100% bullet stuff
btDynamicsWorld* OsgbDice::initBtPhysicsWorld()
{
    btDefaultCollisionConfiguration * collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher * dispatcher = new btCollisionDispatcher( collisionConfiguration );
    btConstraintSolver * solver = new btSequentialImpulseConstraintSolver;

    btVector3 worldAabbMin( -1000, -1000, -1000 );
    btVector3 worldAabbMax( 1000, 1000, 1000 );
    btBroadphaseInterface * inter = new btAxisSweep3( worldAabbMin, worldAabbMax, 1000 );

    btDynamicsWorld * dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, inter, solver, collisionConfiguration );

    dynamicsWorld->setGravity( btVector3(0, 0, -9.8) );

    return( dynamicsWorld );
}

void OsgbDice::initialize()
{
    osg::Group* root = new osg::Group;

	die1 = makeDie( myWorld );
	die2 = makeDie( myWorld );
    //root->addChild( makeDie( myWorld ) );
    //root->addChild( makeDie( myWorld ) );

	root->addChild(die1);
	root->addChild(die2);
	for (int i=0;i<2;i++)
	{
		printf("%d: (%d, %d, %d)\n", i+1, 
		root->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().x(),
		root->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().y(),
		root->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().z());
	}

    /* BEGIN: Create environment boxes */
    float xDim( 10. );
    float yDim( 10. );
    float zDim( 6. );
    float thick( .1 );

    myShakeBox = new osg::MatrixTransform;
    btCompoundShape* compoundShape = new btCompoundShape;
    { // floor -Z (far back of the shake cube)
        osg::Vec3 halfLengths( xDim*.5, yDim*.5, thick*.5 );
        osg::Vec3 center( 0., 0., -zDim*.5 );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // top +Z (invisible, to allow user to see through; no OSG analogue
        osg::Vec3 halfLengths( xDim*.5, yDim*.5, thick*.5 );
        osg::Vec3 center( 0., 0., zDim*.5 );
        //myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // left -X
        osg::Vec3 halfLengths( thick*.5, yDim*.5, zDim*.5 );
        osg::Vec3 center( -xDim*.5, 0., 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // right +X
        osg::Vec3 halfLengths( thick*.5, yDim*.5, zDim*.5 );
        osg::Vec3 center( xDim*.5, 0., 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // bottom of window -Y
        osg::Vec3 halfLengths( xDim*.5, thick*.5, zDim*.5 );
        osg::Vec3 center( 0., -yDim*.5, 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    { // top of window Y
        osg::Vec3 halfLengths( xDim*.5, thick*.5, zDim*.5 );
        osg::Vec3 center( 0., yDim*.5, 0. );
        myShakeBox->addChild( osgBox( center, halfLengths ) );
        btBoxShape* box = new btBoxShape( osgbCollision::asBtVector3( halfLengths ) );
        btTransform trans; trans.setIdentity();
        trans.setOrigin( osgbCollision::asBtVector3( center ) );
        compoundShape->addChildShape( trans, box );
    }
    /* END: Create environment boxes */

    myShakeMotion = new osgbDynamics::MotionState();
    myShakeMotion->setTransform( myShakeBox );
	//myShakeMotion->setWorldTransform( myShakeBox );
    btScalar mass( 0.0 );
    btVector3 inertia( 0, 0, 0 );
    btRigidBody::btRigidBodyConstructionInfo rb( mass, myShakeMotion, compoundShape, inertia );
    myShakeBody = new btRigidBody( rb );
    myShakeBody->setCollisionFlags( myShakeBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
    myShakeBody->setActivationState( DISABLE_DEACTIVATION );
    myWorld->addRigidBody( myShakeBody );

	//printf("=====\n=====\nnum of collision objects: %d\n=====\n=====\n",myWorld->getNumCollisionObjects());

    // Create an omegalib scene node and attach the osg node to it. This is used to interact with the 
    // osg object through omegalib interactors.
    myOso = new OsgSceneObject(myShakeBox);
	root->addChild( myOso->getTransformedNode() );
    mySceneNode = new SceneNode(getEngine());
    mySceneNode->addComponent(myOso);
    mySceneNode->setBoundingBoxVisible(true);
    //mySceneNode->setBoundingBoxVisible(false);
    getEngine()->getScene()->addChild(mySceneNode);
	getEngine()->getDefaultCamera()->setPosition(0,0,30);

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

    /*/ create viewer (using osg funcdtions to view the scene and handle event, as in osgb)
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 150, 150, 400, 400 );
    viewer.setSceneData( root );
    viewer.getCamera()->setViewMatrixAsLookAt(
        osg::Vec3( 0, 0, -20 ), osg::Vec3( 0, 0, 0 ), osg::Vec3( 0, 1, 0 ) );
    viewer.getCamera()->setProjectionMatrixAsPerspective( 40., 1., 1., 50. );
    viewer.addEventHandler( new ShakeManipulator( shakeMotion ) );

    viewer.realize();
    double prevSimTime = 0.;
    while( !viewer.done() )
    {
        const double currSimTime = viewer.getFrameStamp()->getSimulationTime();
        double elapsed( currSimTime - prevSimTime );
        if( viewer.getFrameStamp()->getFrameNumber() < 3 )
            elapsed = 1./60.;
        //osg::notify( osg::ALWAYS ) << elapsed / 3. << ", " << 1./180. << std::endl;
        bulletWorld->stepSimulation( elapsed, 4, elapsed/4. );
        prevSimTime = currSimTime;
        viewer.frame();
    }
    //*/
}

void OsgbDice::update(const UpdateContext& context)
{
	printf("Scene Node: (%lf, %lf, %lf)\n", mySceneNode->getPosition().x(), mySceneNode->getPosition().y(), mySceneNode->getPosition().z());
	/*/ 
	printf("bullet:\n");
	for (int i=0;i<2;i++)
	{
		printf("dice %d: (worldtrans) (%lf, %lf, %lf)\n", i+1,
			myWorld->getCollisionObjectArray().at(i)->getWorldTransform().getOrigin().x(),
			myWorld->getCollisionObjectArray().at(i)->getWorldTransform().getOrigin().y(),
			myWorld->getCollisionObjectArray().at(i)->getWorldTransform().getOrigin().z());
	}
	{
		printf("box: (worldtrans) (%lf, %lf, %lf)\n",
			myWorld->getCollisionObjectArray().at(2)->getWorldTransform().getOrigin().x(),
			myWorld->getCollisionObjectArray().at(2)->getWorldTransform().getOrigin().y(),
			myWorld->getCollisionObjectArray().at(2)->getWorldTransform().getOrigin().z());
	}
	//*/
	
	osg::MatrixTransform* shakeTrans = myShakeMotion->getTransform()->asMatrixTransform();
	btVector3 btTrans(shakeTrans->getMatrix().getTrans().x(), shakeTrans->getMatrix().getTrans().y(), -0.25);

	btTransform world;
    myShakeMotion->getWorldTransform( world );
	//btVector3 o = world.getOrigin();
	//o[ 2 ] = -0.25;
	//world.setOrigin( o );
	world.setOrigin( btTrans );
    myShakeMotion->setWorldTransform( world );

	/*/
	printf("motion:\n");
	{
		printf("motion (worldtrans): (%lf, %lf, %lf)\n",
			world.getOrigin().x(), world.getOrigin().y(), world.getOrigin().z());
	}
	{
		printf("motion (trans): (%lf, %lf, %lf)\n",
			myShakeMotion->getTransform()->asMatrixTransform()->getMatrix().getTrans().x(),
			myShakeMotion->getTransform()->asMatrixTransform()->getMatrix().getTrans().y(),
			myShakeMotion->getTransform()->asMatrixTransform()->getMatrix().getTrans().z());
	}
	//*/

	/*/
	printf("osg:\n");
	for (int i=0;i<2;i++)
	{
		printf("dice %d: (myOsg.child(i)) (%lf, %lf, %lf)\n", i+1,
			myOsg->getRootNode()->asGroup()->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().x(),
			myOsg->getRootNode()->asGroup()->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().y(),
			myOsg->getRootNode()->asGroup()->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans().z());
	}
	{
		printf("box: (myOsg.child(i)) (%lf, %lf, %lf)\n",
			myOsg->getRootNode()->asGroup()->getChild(2)->asTransform()->asMatrixTransform()->getMatrix().getTrans().x(),
			myOsg->getRootNode()->asGroup()->getChild(2)->asTransform()->asMatrixTransform()->getMatrix().getTrans().y(),
			myOsg->getRootNode()->asGroup()->getChild(2)->asTransform()->asMatrixTransform()->getMatrix().getTrans().z());
	}
	//*/

	/*/
	printf("omegalib:\n");
	printf("box: (%lf, %lf, %lf)\n",
		myOso->getTransformedNode()->getMatrix().getTrans().x(),
		myOso->getTransformedNode()->getMatrix().getTrans().y(),
		myOso->getTransformedNode()->getMatrix().getTrans().z());
	//*/

	/*/
	printf("  1: (%d, %d, %d)\n", // strange, strange
		die1->getMatrix().getTrans().x(),
		die1->getMatrix().getTrans().y(),
		die1->getMatrix().getTrans().z());
	printf("  2: (%d, %d, %d)\n", // strange, strange
		die2->getMatrix().getTrans().x(),
		die2->getMatrix().getTrans().y(),
		die2->getMatrix().getTrans().z());
	//*/

	double elapsed = 1./60.;
	myWorld->stepSimulation( elapsed, 4, elapsed/4. );
	//myWorld->stepSimulation( 1./60., 1, 1./60. );
	//prevSimTime = currSimTime;
}

/** \page diceexample The Mandatory Dice Example
No physics-based project would be complete without a dice example. Use the
left mouse button to chake the dice shaker.
*/
int main(int argc, char** argv)
{
	Application<OsgbDice> app("osgbDice");
    return omain(app, argc, argv);
}