###############################################################
#
# Random Number Example - 2013
# Thomas Marrinan
#
# This example demonstrates how to syncronize random number
# generation across multiple nodes of a cluster
#
###############################################################

from math import *
from euclid import *
from omega import *
from cyclops import *

import time
import random


scene = getSceneManager()
scene.setBackgroundColor(Color(0.2, 0.2, 0.2, 1.0))

cam = getDefaultCamera()

light1 = Light.create()
light1.setLightType(LightType.Directional)
light1.setLightDirection(Vector3(0.5, 0.5, 1.0))
light1.setColor(Color(1.0, 1.0, 1.0, 1.0))
light1.setAmbient(Color(0.2, 0.2, 0.2, 1.0))
light1.setEnabled(True)

initRandom = False
sceneLoaded = False

objectList = []

if isMaster():
	mySeed = time.time()
	broadcastCommand("setRandomNumberSeed(%d)" % (mySeed))
	
###################################################################################
###################################################################################

def loadScene():
	global sceneLoaded
	global objectList
	 
	for i in range(100):
		x = random.uniform(-8.0, 8.0)
		y = random.uniform(-1.0, 3.0)
		z = random.uniform(-8.0, 8.0)
		scale = random.uniform(0.05, 0.50)
		red = random.random()
		green = random.random()
		blue = random.random()
	
		obj = SphereShape.create(1, 2)
		obj.setPosition(Vector3(x, y, z))
		obj.setScale(Vector3(scale, scale, scale))
		obj.getMaterial().setProgram('colored')
		obj.getMaterial().setColor(Color(red, green, blue, 1.0), Color(0.0, 0.0, 0.0, 1.0))
		
		objectList.append(obj)
	
	sceneLoaded = True

###################################################################################
###################################################################################

def setRandomNumberSeed(seed):
	global initRandom
	
	random.seed(seed)
	initRandom = True

###################################################################################
###################################################################################

def onUpdate(frame, t, dt):
	global initRandom
	global sceneLoaded
	global objectList
	
	if initRandom and not sceneLoaded:
		loadScene()
	
	rspeed = radians(15.0) # spin 15 degrees every second
	
	for i in range(len(objectList)):
		pos = objectList[i].getPosition()
		radius = sqrt(pos.x*pos.x + pos.z*pos.z)
		theta = atan2(pos.z, pos.x) + rspeed*dt
		objectList[i].setPosition(Vector3(radius*cos(theta), pos.y, radius*sin(theta)))

###################################################################################
###################################################################################

def handleEvent():
	e = getEvent()

###################################################################################
###################################################################################

setUpdateFunction(onUpdate)
setEventFunction(handleEvent)
