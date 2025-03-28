# SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
# SPDX-License-Identifier: GPL-3.0-only

param([switch] $Force)

. "$PSScriptRoot\Version.ps1"

# compile happens in bin\Configuration folder
$vcsRev = git describe --tags --long --dirty --always
$wixFile = "$pwd\..\..\..\Version.wxi"
$wixOld = Get-Content -ErrorAction Ignore -Raw $wixFile
$wixNew = `
@"
<?xml version="1.0" encoding="utf-8"?>
<Include>
  <?define fileVersion="$($fileVersion[0]).$($fileVersion[1]).$($fileVersion[2]).$($fileVersion[3])" ?>
  <?define productVersion="$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])" ?>
  <?define vcsRev="$vcsRev" ?>
</Include>

"@
if ($Force -or ($wixOld -ne $wixNew)) {
    Write-Output "Updating Wix version include: $vcsRev"
    Set-Content -Path $wixFile -Value $wixNew -NoNewline
}
