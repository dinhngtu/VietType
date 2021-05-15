$fileVersion = 1,3,8,0
$registrarFileVersion = 1,0,0,0
$productVersion = 1,3,8,0

if ($Env:GITHUB_ACTIONS -eq "true") {
    $fileVersion[3] = [int]$Env:GITHUB_RUN_NUMBER
    $registrarFileVersion[3] = [int]$Env:GITHUB_RUN_NUMBER
    $productVersion[3] = [int]$Env:GITHUB_RUN_NUMBER
}
