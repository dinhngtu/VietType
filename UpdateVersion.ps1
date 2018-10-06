$vcsFile = "VersionVCS.h"
$vcsOld = Get-Content -ErrorAction Ignore -Raw $vcsFile
$vcsRev = hg id -i
$vcsNew = "#pragma once`n`n#define VCS_REVISION L""$vcsRev""`n"
if ($vcsOld -ne $vcsNew) {
    echo "Updating revision: $vcsRev"
    [System.IO.File]::WriteAllText($vcsFile, [string]::Join("`n", $vcsNew))
}

Get-Content "$PSScriptRoot\Version.ps1" | Invoke-Expression

$hFile = "VersionNumbers.h"
$hOld = Get-Content -ErrorAction Ignore -Raw $hFile
$hNew = `
@"
#pragma once

#define FILEVERSION_TUPLE $($fileVersion[0]),$($fileVersion[1]),$($fileVersion[2]),$($fileVersion[3])
#define FILEVERSION_TEXT "$($fileVersion[0]).$($fileVersion[1]).$($fileVersion[2]).$($fileVersion[3])"

#define PRODUCTVERSION_TUPLE $($productVersion[0]),$($productVersion[1]),$($productVersion[2]),$($productVersion[3])
#define PRODUCTVERSION_TEXT "$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])"

"@
if ($hOld -ne $hNew) {
    echo "Updating version header"
    [System.IO.File]::WriteAllText($hFile, [string]::Join("`n", $hNew))
}