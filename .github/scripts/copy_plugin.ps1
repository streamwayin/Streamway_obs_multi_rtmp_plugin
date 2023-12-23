# Source and destination directories
$sourceDir = "C:\Users\ramji\projects\Streamway_obs_multi_rtmp_plugin\release\obs-plugins"
$destDir = "C:\Program Files\obs-studio\obs-plugins\64bit"

# Enumerate files in the source directory
$files = Get-ChildItem -Path $sourceDir -File

# Function to copy and replace files
function CopyAndReplaceFile($sourceFile, $destFile) {
    # Check if destination file exists
    if (Test-Path $destFile) {
        # Backup the existing file
        Write-Host "Backing up existing file: $destFile"
        Backup-Item -Path $destFile -Destination $destFile.bak
    }

    # Copy the source file to the destination
    Write-Host "Copying file: $sourceFile to $destFile"
    Copy-Item -Path $sourceFile -Destination $destFile
}

# Loop through each file and copy/replace
foreach ($file in $files) {
    $sourceFile = Join-Path $sourceDir $file.Name
    $destFile = Join-Path $destDir $file.Name
    CopyAndReplaceFile $sourceFile $destFile
}

Write-Host "Completed copying and replacing files"
