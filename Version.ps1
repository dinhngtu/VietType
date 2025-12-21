$fileVersion = 1, 11, 13, 0
$productVersion = 1, 11, 13, 0

if ($Env:GITHUB_ACTIONS -eq "true") {
    $fileVersion[3] = [int]$Env:GITHUB_RUN_NUMBER
    $productVersion[3] = [int]$Env:GITHUB_RUN_NUMBER

    if ($Env:GITHUB_REF_TYPE -ieq "tag" -and $Env:GITHUB_REF_NAME -match '^v(\d+)(?:\.(\d+))?(?:\.(\d+))?(?:\.(\d+))?$') {
        $fileVersion[0] = [int]$Matches[1]
        $productVersion[0] = [int]$Matches[1]
        if ($null -ne $Matches[2]) {
            $fileVersion[1] = [int]$Matches[2]
            $productVersion[1] = [int]$Matches[2]
        }
        if ($null -ne $Matches[3]) {
            $fileVersion[2] = [int]$Matches[3]
            $productVersion[2] = [int]$Matches[3]
        }
        if ($null -ne $Matches[4]) {
            $fileVersion[3] = [int]$Matches[4]
            $productVersion[3] = [int]$Matches[4]
        }
    }
}
