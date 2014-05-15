# This script is executed when the omegalib python interpreter (orun) starts, 
# before running application scripts.
# It sets up a system menu with several config options for sound, navigation, 
# graphics etc. If you want to change (or remove) the default menu, use the  
# orun/initScript config option to specify a new or an empty script.
# You can also change the orun/appStartFunction config option to change the 
# entry point into this script (by default it is set to _onAppStart)
from omega import *
from euclid import *

speedLabel = None
mainmnu = None

# add the current path to the data search paths.
import os
addDataPath(os.getcwd())

def _setCamSpeed(speedLevel):
	global speedLabel
	s = 10 ** (speedLevel - 4)
	speedLabel.setText("Navigation Speed: " + str(s) + "x")
	cc = getDefaultCamera().getController()
	if(cc != None):
		cc.setSpeed(s)

def _resetCamera():
	getDefaultCamera().setPosition(Vector3(0, 0, 0))
	getDefaultCamera().setPitchYawRoll(Vector3(0, 0, 0))
		
def _autonearfar(value):
	if(value):
		queueCommand(":autonearfar on")
	else:
		queueCommand(":autonearfar off")

def _displayWand(value):
	if(value):
		queueCommand("getSceneManager().displayWand(0, 1)")
	else:
		queueCommand("getSceneManager().hideWand(0)")
		
def _setSoundServerVolume( value ):
	newVolume = value - 30
	globalVolumeLabel.setText("Global Volume: " + str(newVolume) )
	soundEnv.setServerVolume(newVolume)
	
def _onAppStart():
	global mainmnu
    # mm = getViewer().getMenuManager()
	mm = MenuManager.createAndInitialize()
	mainmnu = mm.createMenu("Main Menu")

	# If menus are in 2d mode, add a menu open button
	if(not getBoolSetting('config/ui', 'menu3dEnabled', False)):
		uim = UiModule.instance()
		wf = uim.getWidgetFactory()
		mainButton = wf.createButton('mainButton', uim.getUi())
		mainButton.setText("Main Menu")
		mainButton.setUIEventCommand('mainmnu.show()')
		mainButton.setStyleValue('fill', 'black')
		mainmnu.getContainer().setPosition(Vector2(5, 25))
	
	mi = mainmnu.addImage(loadImage("omegalib-transparent-white.png"))
	ics = mi.getImage().getSize() * 0.1
	mi.getImage().setSize(ics)
	
	mm.setMainMenu(mainmnu)
	sysmnu = mainmnu.addSubMenu("System")
	mi = sysmnu.addButton("Toggle freefly", ":freefly")
	mi.getButton().setCheckable(True)
	mi = sysmnu.addButton("Reset", "_resetCamera()")
	mi = sysmnu.addButton("Auto Near / Far", "_autonearfar(%value%)")
	mi.getButton().setCheckable(True)
	mi = sysmnu.addButton("Display Wand", "_displayWand(%value%)")
	mi.getButton().setCheckable(True)
	global speedLabel
	speedLabel = sysmnu.addLabel("sd")
	_setCamSpeed(4)
	ss = sysmnu.addSlider(10, "_setCamSpeed(%value%)")
	ss.getSlider().setValue(4)
	ss.getWidget().setWidth(200)
	
	if( isSoundEnabled() ):
		global soundEnv
		global serverVolume
		global globalVolumeLabel
			
		soundEnv = getSoundEnvironment()
		serverVolume = soundEnv.getServerVolume();
	
		globalVolumeLabel = sysmnu.addLabel("Global Volume: --")
		
		value = serverVolume + 30
		globalVolumeLabel.setText("Global Volume: " + str(serverVolume))
	
		ss = sysmnu.addSlider(39, "_setSoundServerVolume(%value%)")
		ss.getSlider().setValue(30)
		ss.getWidget().setWidth(200)
		
		ss.getSlider().setValue(value)
		
	mi = sysmnu.addButton("Enable Stereo", "toggleStereo()")
	mi.getButton().setCheckable(True)
	mi.getButton().setChecked(isStereoEnabled())
	
	mi = sysmnu.addButton("Toggle Console", ":c")
	mi = sysmnu.addButton("List Active Modules", "printModules()")
	mi = sysmnu.addButton("Exit omegalib", "_shutdown()")


shuttingDown = False
fadeOutVal = 0

def _shutdown():
	global shuttingDown
	global fadeOutVal
	shuttingDown = True
	fadeOutVal = 0

def onUpdate(frame, t, dt):
	global shuttingDown
	global fadeOutVal
	if(shuttingDown):
		if(fadeOutVal >= 1): oexit()
		else:
			uim = UiModule.instance()
			ui = uim.getUi()
			alpha = int(fadeOutVal * 256)
			ui.setStyleValue('fill', '#000000' + hex(alpha)[2:])
			fadeOutVal += dt
	
setUpdateFunction(onUpdate)
