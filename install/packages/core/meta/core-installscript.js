function Component()
{
}

Component.prototype.createOperations = function(archive)
{
    component.createOperations(archive);

	if(typeof systemInfo != "undefined" && systemInfo.kernelType != "winnt") {
		// Create links to executable and data in /usr
		component.addElevatedOperation("Execute", "ln", "-s", "@TargetDir@/bin/orun", "/usr/bin/orun", 
			"UNDOEXECUTE", "rm", "/usr/bin/orun");
		component.addElevatedOperation("Execute", "ln", "-s", "@TargetDir@", "/usr/share/omegalib", 
			"UNDOEXECUTE", "rm", "/usr/share/omegalib");
    }
}