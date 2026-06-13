var SHELL_REGISTRAR_RELATIVE_PATH = "/InstallerTools/ShellExtensionRegistrar.exe";

function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    // Preview mode (IFW_PREVIEW): registrar helper is absent, skip registration.
    if (installer.environmentVariable("IFW_PREVIEW") !== "") {
        return;
    }

    if (systemInfo.productType !== "windows") {
        return;
    }

    var targetDir = installer.toNativeSeparators(installer.value("TargetDir"));
    var helperPath = installer.toNativeSeparators(installer.value("TargetDir") + SHELL_REGISTRAR_RELATIVE_PATH);

    component.addElevatedOperation("Execute",
        helperPath,
        "apply",
        targetDir,
        "UNDOEXECUTE",
        helperPath,
        "remove",
        targetDir);
}
