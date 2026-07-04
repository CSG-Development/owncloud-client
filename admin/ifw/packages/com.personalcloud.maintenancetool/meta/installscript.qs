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

    component.installerbaseBinaryPath = "@TargetDir@/PersonalCloudMaintenanceTool.app";
    installer.setInstallerBaseBinary(component.installerbaseBinaryPath);
}
