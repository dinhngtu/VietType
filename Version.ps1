$fileVersion = 1,9,4,0
$productVersion = 1,9,4,0

if ($Env:GITHUB_ACTIONS -eq "true") {
    $fileVersion[3] = [int]$Env:GITHUB_RUN_NUMBER
    $productVersion[3] = [int]$Env:GITHUB_RUN_NUMBER
}
