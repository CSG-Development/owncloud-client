function Component()
{
    component.ifwVersion = installer.value("FrameworkVersion");
    installer.installationStarted.connect(this, Component.prototype.onInstallationStarted);
}

Component.prototype.onInstallationStarted = function()
{
    if (!component.installationRequested() && !component.updateRequested()) {
        return;
    }

    if (installer.value("os") !== "mac") {
        return;
    }

    var updateResourcePath = installer.value("TargetDir");
    if (installer.versionMatches(component.ifwVersion, "<4.8.0")) {
        component.installerbaseBinaryPath = "@TargetDir@/PersonalCloudMaintenanceTool.app";
    } else {
        updateResourcePath += "/tmpMaintenanceToolApp";
        component.installerbaseBinaryPath = "@TargetDir@/tmpMaintenanceToolApp/PersonalCloudMaintenanceTool.app";
    }

    installer.setInstallerBaseBinary(component.installerbaseBinaryPath);
    installer.setValue("DefaultResourceReplacement", updateResourcePath + "/update.rcc");
}

Component.prototype.createOperationsForArchive = function(archive)
{
    if (installer.versionMatches(component.ifwVersion, "<4.8.0") || installer.value("os") !== "mac") {
        component.createOperationsForArchive(archive);
        return;
    }

    component.addOperation("Extract", archive, "@TargetDir@/tmpMaintenanceToolApp");
}
