# build.ps1
# This script configures and builds the Vocal Lab JUCE plugin

Write-Host "Configuring CMake Project..."
cmake -B build -S .

if ($LASTEXITCODE -eq 0) {
    Write-Host "Building Release version..."
    cmake --build build --config Release
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build Successful!"
        Write-Host "You can find your Standalone app in: .\build\VocalLab_artefacts\Release\Standalone"
        Write-Host "You can find your VST3 plugin in: .\build\VocalLab_artefacts\Release\VST3"
    } else {
        Write-Host "Build failed."
    }
} else {
    Write-Host "CMake configuration failed. Please ensure CMake and Visual Studio (with C++ Desktop development workload) are installed."
}
