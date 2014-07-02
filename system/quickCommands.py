addQuickCommand('rc', '_resetCamera()', 0, 'resets the default camera to the original position / orientation')
addQuickCommand('q', 'oexit()', 0, 'exit omegalib')


# Functions used by some of the quick commands
def _resetCamera():
	getDefaultCamera().setPosition(Vector3(0, 0, 0))
	getDefaultCamera().setPitchYawRoll(Vector3(0, 0, 0))