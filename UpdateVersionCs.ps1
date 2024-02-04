# SPDX-FileCopyrightText: Copyright (c) 2022 Dinh Ngoc Tu
# SPDX-License-Identifier: GPL-3.0-only

param([switch] $Force)

$vcsRev = git describe --tags --long --dirty --always
Get-Content -Raw "$PSScriptRoot\Version.ps1" | Invoke-Expression
$productVersionText = "$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])"

$csFile = "$pwd\..\..\Properties\Version.cs"
$csOld = Get-Content -ErrorAction Ignore -Raw $csFile
$csNew = `
@"
using System.Reflection;

[assembly: AssemblyVersion("$productVersionText")]
[assembly: AssemblyFileVersion("$productVersionText")]
[assembly: AssemblyInformationalVersion("$productVersionText")]

namespace VietTypeConfig {
    internal static class Version {
        public const string ProductVersion = "$productVersionText";
        public const string VcsRevision = "$vcsRev";
    }
}
"@
if ($Force -or ($csOld -ne $csNew)) {
    echo "Updating Version.cs"
    [System.IO.File]::WriteAllText($csFile, [string]::Join("`n", $csNew))
}
