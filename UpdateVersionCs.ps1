param([switch] $Force)

Get-Content -Raw "$PSScriptRoot\Version.ps1" | Invoke-Expression

$csFile = "..\..\Properties\Version.cs"
$csOld = Get-Content -ErrorAction Ignore -Raw $csFile
$csNew = `
@"
using System.Reflection;

[assembly: AssemblyInformationalVersion("$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])")]
"@
if ($Force -or ($csOld -ne $csNew)) {
    echo "Updating Version.cs"
    [System.IO.File]::WriteAllText($csFile, [string]::Join("`n", $csNew))
}
