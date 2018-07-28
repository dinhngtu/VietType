$file = "VersionVCS.h"
$oldver = Get-Content -ErrorAction Ignore -First 1 $file
$newver = "#define VCS_REVISION L""$(hg id -i)"""
if ($oldver -ne $newver) {
    echo "Updating version: $newver"
    $newver > $file
}