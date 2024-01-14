# SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
# SPDX-License-Identifier: GPL-3.0-only

param([switch] $Force)

Get-Content -Raw "$PSScriptRoot\Version.ps1" | Invoke-Expression

# compile happens in bin\Configuration folder
$vcsRev = git describe --tags --long --dirty --always
$wixFile = "..\..\Version.wxi"
$wixOld = Get-Content -ErrorAction Ignore -Raw $wixFile
$wixNew = `
@"
<?xml version="1.0" encoding="utf-8"?>
<Include>
  <?define productVersion="$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])" ?>
  <?define vcsRev="$vcsRev" ?>
</Include>

"@
if ($Force -or ($wixOld -ne $wixNew)) {
    echo "Updating Wix version include: $vcsRev"
    [System.IO.File]::WriteAllText($wixFile, [string]::Join("`n", $wixNew))
}
