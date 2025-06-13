if ($IsWindows) {
    $python = (Get-Item "C:\hostedtoolcache\windows\Python\3.11*\x64\python.exe").FullName
} elseif ($IsMacOS) {
    $python = (Get-Command "python3.11").Source
} else {
    $python = (Get-Command python3).Source
}

$RepoRoot = "{0}/../../" -f ([System.IO.Path]::GetDirectoryName($myInvocation.MyCommand.Definition))
$craftMasterPath = if ($IsWindows) { "${env:USERPROFILE}/craft/CraftMaster/CraftMaster.py" } else { "${env:HOME}/craft/CraftMaster/CraftMaster.py" }
$workspacePath = if ($IsWindows) { "${env:USERPROFILE}/craft" } else { "${env:HOME}/craft" }

$command = @($craftMasterPath,
             "--config", "${RepoRoot}/.craft.ini",
             "--config-override", "${RepoRoot}/.github/workflows/craft_override.ini",
             "--target", "${env:CRAFT_TARGET}",
             "--variables", "WORKSPACE=$workspacePath") + $args

Write-Host "Exec: ${python} ${command}"

& $python @command
if ($LASTEXITCODE -ne 0) {
    exit 1
}
