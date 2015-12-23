function Component()
{
}

Component.prototype.createOperations = function(archive)
{
    component.createOperations(archive);

	if(typeof systemInfo != "undefined" && systemInfo.kernelType != "winnt") {
		// Create links to executable and data in /usr
		component.addOperation("Execute", "ln", "-s", "@TargetDir@/bin/orun", "/usr/local/bin/orun", 
			"UNDOEXECUTE", "rm", "/usr/local/bin/orun");
    }
}