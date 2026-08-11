param(
    [ValidateSet("pico", "pico_w", "waveshare_rp2040_zero", "pico2")]
    [string]$Board = "pico",

    [switch]$ElfOnly,

    [string]$PicoSdkPath
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot ("build\{0}" -f $Board)

if ($PicoSdkPath) {
    $resolvedSdkPath = (Resolve-Path -LiteralPath $PicoSdkPath).Path
    $env:PICO_SDK_PATH = $resolvedSdkPath
}

if (-not $env:PICO_SDK_PATH) {
    throw "PICO_SDK_PATH is not set. Pass -PicoSdkPath or set it to Pico SDK 2.1.1."
}

$sdkImport = Join-Path $env:PICO_SDK_PATH "external\pico_sdk_import.cmake"
if (-not (Test-Path -LiteralPath $sdkImport)) {
    throw "PICO_SDK_PATH does not contain external\pico_sdk_import.cmake: $env:PICO_SDK_PATH"
}

if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    throw "Ninja is required and was not found in PATH."
}

$compiler = if ($Board -eq "pico2") {
    "pico_arm_cortex_m33_gcc"
} else {
    "pico_arm_cortex_m0plus_gcc"
}

$configureArguments = @(
    "-S", $projectRoot,
    "-B", $buildDirectory,
    "-G", "Ninja",
    "-DPICO_BOARD=$Board",
    "-DPICO_COMPILER=$compiler"
)

$configureArguments += if ($ElfOnly) {
    "-DPICO_NO_PICOTOOL=1"
} else {
    "-DPICO_NO_PICOTOOL=0"
}

Write-Host "Configuring board '$Board' with Pico SDK '$env:PICO_SDK_PATH'..."
& cmake @configureArguments
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& cmake --build $buildDirectory
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$elfPath = Join-Path $buildDirectory "host.elf"
if (-not (Test-Path -LiteralPath $elfPath)) {
    throw "Build completed without the expected ELF: $elfPath"
}

Write-Host "Build succeeded: $elfPath"
if ($ElfOnly) {
    Write-Host "ELF-only mode was used; UF2 generation requires Pico SDK 2.1.1-compatible picotool."
} else {
    $uf2Path = Join-Path $buildDirectory "host.uf2"
    if (-not (Test-Path -LiteralPath $uf2Path)) {
        throw "UF2 was not generated. Install a Pico SDK 2.1.1-compatible picotool or use -ElfOnly."
    }
    Write-Host "UF2: $uf2Path"
}
