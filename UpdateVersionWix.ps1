param([switch] $Force)

Get-Content "$PSScriptRoot\Version.ps1" | Invoke-Expression

# compile happens in bin\Configuration folder
$vcsRev = git describe --tags --dirty --always
$wixFile = "..\..\Version.wxi"
$wixOld = Get-Content -ErrorAction Ignore -Raw $wixFile
$wixNew = `
@"
<?xml version="1.0" encoding="utf-8"?>
<Include>
  <?define productVersion="$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])" ?>
  <?define vcsRev="Rev-$vcsRev" ?>
</Include>

"@
if ($Force -or ($wixOld -ne $wixNew)) {
    echo "Updating Wix version include: $vcsRev"
    [System.IO.File]::WriteAllText($wixFile, [string]::Join("`n", $wixNew))
}
