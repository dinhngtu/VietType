Get-Content "$PSScriptRoot\Version.ps1" | Invoke-Expression

# compile happens in bin\Configuration folder
$wixFile = "..\..\Version.wxi"
$wixOld = Get-Content -ErrorAction Ignore -Raw $wixFile
$wixNew = `
@"
<?xml version="1.0" encoding="utf-8"?>
<Include>
  <?define fileVersion="$($fileVersion[0]).$($fileVersion[1]).$($fileVersion[2]).$($fileVersion[3])" ?>
  <?define productVersion="$($productVersion[0]).$($productVersion[1]).$($productVersion[2]).$($productVersion[3])" ?>
</Include>

"@
if ($wixOld -ne $wixNew) {
    echo "Updating Wix version include"
    [System.IO.File]::WriteAllText($wixFile, [string]::Join("`n", $wixNew))
}