$fileVersion = 1,7,0,0
$productVersion = 1,7,0,0

if ($Env:GITHUB_ACTIONS -eq "true") {
    $fileVersion[2] = [int]$Env:GITHUB_RUN_NUMBER
    $productVersion[2] = [int]$Env:GITHUB_RUN_NUMBER
}
