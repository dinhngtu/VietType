# SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
# SPDX-License-Identifier: GPL-3.0-only

param([switch] $Force)

$vcsFile = "VersionVCS.h"
$vcsOld = Get-Content -ErrorAction Ignore -Raw $vcsFile
$vcsRev = git describe --long --dirty --always --abbrev=12
$vcsNew = "#pragma once`n`n#define VCS_REVISION L""$vcsRev""`n"
if ($Force -or ($vcsOld -ne $vcsNew)) {
    Write-Output "Updating revision: $vcsRev"
    Set-Content -Path $vcsFile -Value $vcsNew -NoNewline
}

. "$PSScriptRoot\Version.ps1"

$hFile = "$pwd\VersionNumbers.h"
$hOld = Get-Content -ErrorAction Ignore -Raw $hFile
$hNew = `
@"
#pragma once

#define FILEVERSION_TUPLE $($fileVersion[0]),$($fileVersion[1]),$($fileVersion[2]),$($fileVersion[3])
#define FILEVERSION_TEXT "$($fileVersion[0]).$($fileVersion[1]).$($fileVersion[2]).$($fileVersion[3])"

#define PRODUCTVERSION_TUPLE $($productVersion[0]),$($productVersion[1]),$($productVersion[2]),$($productVersion[3])
#define PRODUCTVERSION_TEXT "$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])"

"@
if ($Force -or ($hOld -ne $hNew)) {
    Write-Output "Updating version header"
    Set-Content -Path $hFile -Value $hNew -NoNewline
}
